#include "Server.hpp"
#include "SocketException.hpp"
#include "KapMirror/Runtime/ArraySegment.hpp"
#include "KapMirror/Runtime/NetworkReader.hpp"
#include <iostream>

namespace KapMirror {
    namespace Transports {
        Server::Server() {
        }

        void Server::listen(int port) {
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

                    // TODO: Add thread
                    while (client->connected()) {
                        try {
                            ArraySegment<char> segment = client->receive(1024);
                            std::string msg(segment.toArray(), segment.getSize());
                            std::cout << "Server: received message=" << msg << std::endl;

                            // send back
                            client->send(segment);
                        } catch (SocketException& e) {
                            std::cout << "Server: client disconnected" << std::endl;
                            break;
                        }
                    }
                }

                listener->stop();
            } catch (SocketException& e) {
                std::cout << "Server: " << e.what() << std::endl;
            }
        }

        bool Server::start(int port) {
            std::cout << "Server: Start port=" << port << std::endl;

            // TODO: Add thread
            listen(port);
            return true;
        }

        void Server::stop() {
            std::cout << "Server: stopping..." << std::endl;

            // stop listening to connections
            listener->stop();
        }
    }
}
