#include "Client.hpp"
#include <iostream>
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
}

void Client::connect(std::string ip, int port) {
    if (connecting() || connected()) {
        std::cerr << "Client: Already connecting or connected" << std::endl;
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
        EventType eventType;
        std::shared_ptr<ArraySegment<byte>> message;
        if (receivePipe.pop(connectionId, eventType, message)) {
            switch (eventType) {
                case EventType::Connected:
                    if (onConnected) {
                        onConnected();
                    }
                    break;
                case EventType::Disconnected:
                    if (onDisconnected) {
                        onDisconnected();
                    }
                    dispose();
                    break;
                case EventType::Data:
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

void Client::send(std::shared_ptr<ArraySegment<byte>> message) {
    if (!connected()) {
        std::cerr << "Client: Not connected" << std::endl;
        return;
    }
    if (message->getSize() > maxMessageSize) {
        std::cerr << "Client: Message too large" << std::endl;
        return;
    }
    if (sendPipe.getSize() > sendQueueLimit) {
        std::cerr << "Client: Send queue full" << std::endl;

        // Disconnect if the send queue is full
        disconnect();
        return;
    }

    sendPipe.push(message);
}

void Client::run(std::string ip, int port) {
    auto address = std::make_shared<Address>(ip, port);
    client = std::make_shared<TcpClient>(address);

    try {
        client->connect();
    } catch (SocketException& e) {
        std::cerr << "Client: Error=" << e.what() << std::endl;
        isConnecting = false;
        return;
    }

    isConnecting = false;

    receivePipe.push(0, EventType::Connected);

    byte* buffer = new byte[4 + maxMessageSize];
    while (client->connected() && running) {
        // Don't burn too much CPU while idling
        std::this_thread::sleep_for(std::chrono::microseconds(1));

        if (client->isWritable()) {
            if (!sendPipe.isEmpty()) {
                std::shared_ptr<ArraySegment<byte>> message;
                if (sendPipe.pop(message)) {
                    client->send(message);
                }

                // We need to throttle our connection transmission rate
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Throttle
            }
        }

        // Check for incoming data
        if (client->isReadable()) {
            try {
                // Clear the buffer
                std::memset(buffer, 0, 4 + maxMessageSize);

                int size = 0;
                if (!client->receive(4 + maxMessageSize, buffer, size)) {
                    // Connection closed
                    break;
                }

                auto message = ArraySegment<byte>::createArraySegment(buffer, size);

                // Push the message to the receive pipe
                receivePipe.push(0, EventType::Data, message);

                if (receivePipe.getSize() >= receiveQueueLimit) {
                    std::cerr << "Server: Receive pipe is full, dropping messages" << std::endl;
                    break;
                }
            } catch (SocketException& e) {
                std::cerr << "Client: Exception=" << e.what() << std::endl;
                break;
            }
        }
    }

    // Free buffer
    delete[] buffer;

    receivePipe.push(0, EventType::Disconnected);
}
