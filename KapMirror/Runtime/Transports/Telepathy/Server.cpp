#include "Server.hpp"
#include "SocketException.hpp"
#include <iostream>

KapMirror::Transports::Server::Server() {
}

void KapMirror::Transports::Server::listen(int port) {
    try {
        // start listener
        listener = std::make_shared<TcpListener>(port);

        listener->start();
        std::cout << "Server: listening port=" << port << std::endl;

        // keep accepting new clients
        while (true) {
            // wait and accept new client
            auto client = listener->acceptTcpClient();

            std::cout << "Server: new client" << std::endl;
        }

        listener->stop();
    } catch (SocketException& e) {
        std::cout << "Server: " << e.what() << std::endl;
    }
}
