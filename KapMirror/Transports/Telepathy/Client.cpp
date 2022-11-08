#include "Client.hpp"
#include "KapMirror/Core/NetworkReader.hpp"
#include "KapMirror/Core/NetworkWriter.hpp"
#include "Debug.hpp"
#include <cstring>

using namespace KapMirror::Telepathy;

Client::Client(int _maxMessageSize) : maxMessageSize(_maxMessageSize) {
    running = false;
    isConnecting = false;
}

Client::~Client() {
    if (running) {
        dispose();
    }
}

void Client::dispose() {
    running = false;
    isConnecting = false;
    if (clientThread.joinable()) {
        clientThread.join();
    }
    client->close();
    receivePipe.clear();

    // clean values
    client = nullptr;
}

void Client::connect(const std::string& ip, int port) {
    if (connecting() || connected()) {
        KapEngine::Debug::error(
            "Client: Telepathy Client can not create connection because an existing connection is connecting or connected");
        return;
    }

    if (port < 0 || port > 65535) {
        throw std::runtime_error("Invalid port number");
    }

    running = true;
    isConnecting = true;

    clientThread = std::thread([this, ip, port]() { this->run(ip, port); });
}

void Client::disconnect() {
    if (connecting() || connected()) {
        dispose();

        // Call onDisconnected because the poll loop is not running anymore
        if (onDisconnected) {
            onDisconnected();
        }
    }
}

int Client::tick(int processLimit) {
    if (processLimit < 0) {
        throw std::runtime_error("Invalid process limit");
    }

    if (receivePipe.isEmpty()) {
        return 0;
    }

    for (int i = 0; i < processLimit; i++) {
        int connectionId;
        MagnificentReceivePipe::EventType eventType;
        std::shared_ptr<ArraySegment<byte>> message;
        if (receivePipe.pop(connectionId, eventType, message)) {
            switch (eventType) {
                case MagnificentReceivePipe::EventType::Connected:
                    if (onConnected) {
                        onConnected();
                    }
                    break;
                case MagnificentReceivePipe::EventType::Disconnected:
                    if (onDisconnected) {
                        onDisconnected();
                    }
                    dispose();
                    break;
                case MagnificentReceivePipe::EventType::Data:
                    if (onData) {
                        onData(message);
                    }
                    break;
            }
        } else {
            break;
        }
    }
    return receivePipe.getSize();
}

void Client::send(const std::shared_ptr<ArraySegment<byte>>& message) {
    if (!connected()) {
        KapEngine::Debug::error("Client.Send: not connected!");
        return;
    }
    if (message->getSize() > maxMessageSize) {
        KapEngine::Debug::error("Client.Send: message too big: " + std::to_string(message->getSize()) +
                                ", Limit: " + std::to_string(maxMessageSize));
        return;
    }
    if (sendPipe.getSize() > sendQueueLimit) {
        KapEngine::Debug::error(
            "Client.Send: sendPipe reached limit of " + std::to_string(sendQueueLimit) +
            ". This can happen if we call send faster than the network can process messages. Disconnecting to avoid ever growing "
            "memory & latency.");

        // Disconnect if the send queue is full
        disconnect();
        return;
    }

    sendPipe.push(message);
}

void Client::run(const std::string& ip, int port) {
    auto address = std::make_shared<Address>(ip, port);
    client = std::make_shared<TcpClient>(address);

    try {
        client->connect();
    } catch (SocketException& e) {
        KapEngine::Debug::error("Client: Error=" + std::string(e.what()));
        isConnecting = false;

        receivePipe.push(0, MagnificentReceivePipe::EventType::Disconnected);
        return;
    }

    isConnecting = false;

    receivePipe.push(0, MagnificentReceivePipe::EventType::Connected);

    byte* header = new byte[4];
    byte* buffer = new byte[maxMessageSize];

    while (client->connected() && running) {
        // Don't burn too much CPU while idling
        std::this_thread::sleep_for(std::chrono::microseconds(1));

        if (client->isWritable()) {
            if (!sendPipe.isEmpty()) {
                std::shared_ptr<ArraySegment<byte>> message;
                if (sendPipe.pop(message)) {
                    NetworkWriter writer;
                    // Write the message size
                    writer.write(message->getSize());
                    // Write the message
                    writer.writeArraySegment(message);

                    std::shared_ptr<ArraySegment<byte>> segment = writer.toArraySegment();
                    client->send(segment->toArray(), segment->getSize());
                }

                // We need to throttle our connection transmission rate
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Throttle
            }
        }

        // Check for incoming data
        if (client->isReadable()) {
            try {
                // Clear the buffer
                std::memset(header, 0, 4);
                std::memset(buffer, 0, maxMessageSize);

                // Read the message size
                int size = 0;
                if (!client->receive(4, header, size)) {
                    // Connection closed
                    break;
                }

                NetworkReader reader(header);
                int messageSize = reader.read<int>();

                if (messageSize <= 0 || messageSize > maxMessageSize) {
                    KapEngine::Debug::error("Client: Invalid message size: " + std::to_string(messageSize));
                    break;
                }

                if (!client->receive(messageSize, buffer, size)) {
                    // Connection closed
                    break;
                }

                auto message = ArraySegment<byte>::createArraySegment(buffer, size);

                // Push the message to the receive pipe
                receivePipe.push(0, MagnificentReceivePipe::EventType::Data, message);

                if (receivePipe.getSize() >= receiveQueueLimit) {
                    KapEngine::Debug::error("Client: Receive pipe is full, dropping messages");
                    break;
                }
            } catch (SocketException& e) {
                KapEngine::Debug::error("Client: Exception=" + std::string(e.what()));
                break;
            }
        }
    }

    // Free buffer
    delete[] buffer;

    receivePipe.push(0, MagnificentReceivePipe::EventType::Disconnected);
}
