#include "NetworkServer.hpp"
#include <iostream>

using namespace KapMirror;

NetworkServer::NetworkServer() {
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
            std::cout << "Server: Client connected" << std::endl;

            auto connection = std::make_shared<ClientConnection>();
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

void NetworkServer::handleConnection(std::shared_ptr<ClientConnection> connection) {
    while (connection->client->connected() && running) {
        // Don't burn too much CPU while idling
        std::this_thread::sleep_for(std::chrono::microseconds(1));

        if (connection->client->isWritable()) {
        }

        if (connection->client->isReadable()) {
            try {
                auto message = connection->client->receive(1024);
                std::cout << "Server: Received message Size=" << message.getSize() << std::endl;
            } catch(SocketException& e) {
                std::cout << "Server: Client Error=" << e.what() << std::endl;
                break;
            }
        }
    }

    std::cout << "Server: Client disconnected" << std::endl;
}
