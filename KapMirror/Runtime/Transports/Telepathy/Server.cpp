#include "Server.hpp"
#include <iostream>

using namespace KapMirror::Transports;

Server::Server() {
}

void Server::start(int port) {
    if (port < 0 || port > 65535) {
        throw std::runtime_error("Invalid port number");
    }

    listenerThread = std::thread(&Server::listen, this, port);
}

void Server::listen(int port) {
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

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (!listener->isReadable()) {
            continue;
        }

        try {
            auto client = listener->acceptTcpClient();
            std::cout << "Server: Client connected" << std::endl;

            while (true);
        } catch (SocketException& e) {
            std::cout << "Server: Error=" << e.what() << std::endl;
        }
    }
}
