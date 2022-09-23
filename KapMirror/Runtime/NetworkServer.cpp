#include "NetworkServer.hpp"
#include "ArraySegment.hpp"
#include <iostream>
#include <memory>
#include <cstring>

using namespace KapMirror;

NetworkServer::NetworkServer(int _maxMessageSize) : maxMessageSize(_maxMessageSize) {
}

NetworkServer::~NetworkServer() {
    close();
}

void NetworkServer::close() {
    std::cout << "Server: Closing server" << std::endl;
    running = false;
    if (listenerThread.joinable()) {
        listenerThread.join();
    }
    listener->close();
}

void NetworkServer::start(int port) {
    if (port < 0 || port > 65535) {
        throw std::runtime_error("Invalid port number");
    }

    running = true;
    counter = 0;

    listenerThread = std::thread([this, port]() { this->listen(port); });
}

void NetworkServer::listen(int port) {
    std::cout << "Server: Listening on port " << port << std::endl;

    auto address = std::make_shared<Address>(port);
    listener = std::make_shared<TcpListener>(address);
    listener->setBlocking(false);

    try {
        listener->start();
    } catch (SocketException& e) {
        std::cout << "Server: Error=" << e.what() << std::endl;
        return;
    }

    while (running) {
        if (!listener->isReadable()) {
            // Don't burn too much CPU while idling
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        try {
            auto client = listener->acceptTcpClient();

            // Create a new connection object
            auto connection = std::make_shared<ClientConnection>();
            connection->id = ++counter;
            connection->client = client;
            connection->thread = std::thread([this, connection]() { this->handleConnection(connection); });

            std::lock_guard<std::mutex> lock(connectionListMutex);
            connectionList.push_back(connection);
        } catch (SocketException& e) {
            std::cout << "Server: Error=" << e.what() << std::endl;
        }
    }

    // Wait for all connection threads to finish
    std::lock_guard<std::mutex> lock(connectionListMutex);
    for (auto& connection : connectionList) {
        if (connection->thread.joinable()) {
            connection->thread.join();
        }
    }
    connectionList.clear();
}

int NetworkServer::tick(int processLimit) {
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
                    std::cout << "Server: Client " << connectionId << " connected" << std::endl;
                    break;
                case EventType::Disconnected:
                    std::cout << "Server: Client " << connectionId << " disconnected" << std::endl;
                    disconnectClient(connectionId);
                    break;
                case EventType::Data:
                    std::cout << "Server: Client " << connectionId << " Received message Size=" << message->getSize() << std::endl;
                    break;
            }
        } else {
            break;
        }
    }
    return receivePipe.getSize();
}

void NetworkServer::disconnectClient(int clientId) {
    std::lock_guard<std::mutex> lock(connectionListMutex);
    if (connectionList.empty()) {
        return;
    }
    for (auto it = connectionList.begin(); it != connectionList.end(); it++) {
        std::shared_ptr<ClientConnection> connection = *it;
        if (connection->id == clientId) {
            connection->client->close();
            if (connection->thread.joinable()) {
                connection->thread.join();
            }
            connectionList.erase(it);
            break;
        }
    }
}

void NetworkServer::handleConnection(std::shared_ptr<ClientConnection> connection) {
    receivePipe.push(connection->id, EventType::Connected);

    byte* buffer = new byte[4 + maxMessageSize];
    while (connection->client->connected() && running) {
        // Don't burn too much CPU while idling
        std::this_thread::sleep_for(std::chrono::microseconds(1));

        if (connection->client->isWritable()) {
        }

        if (connection->client->isReadable()) {
            try {
                // Clear the buffer
                std::memset(buffer, 0, 4 + maxMessageSize);

                int size = 0;
                if (!connection->client->receive(4 + maxMessageSize, buffer, size)) {
                    // Connection closed
                    break;
                }

                auto message = ArraySegment<byte>::createArraySegment(buffer, size);

                // Push the message to the receive pipe
                receivePipe.push(connection->id, EventType::Data, message);

                if (receivePipe.getSize() >= receiveQueueLimit) {
                    std::cout << "Server: Receive pipe is full, dropping messages" << std::endl;
                    break;
                }
            } catch(SocketException& e) {
                std::cout << "Server: Client Error=" << e.what() << std::endl;
                break;
            }
        }
    }

    // Free buffer
    delete[] buffer;

    receivePipe.push(connection->id, EventType::Disconnected);
}
