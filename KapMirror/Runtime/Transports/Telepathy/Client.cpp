#include "Client.hpp"
#include "SocketException.hpp"
#include "KapMirror/Runtime/NetworkReader.hpp"
#include <iostream>

namespace KapMirror {
    namespace Transports {
        Client::Client(int _maxMessageSize) : maxMessageSize(_maxMessageSize) {
        }

        // void Client::receiveThreadFunction(std::string host, int port) {
        //     std::cout << "Client: receive thread started" << std::endl;
        //     try {
        //         // connect to server (blocking)
        //         client->connect(host, port);

        //         isConnecting = false;

        //         while (client->connected()) {
        //             auto segment = client->receive(1024);
        //             KapMirror::NetworkReader reader(segment);
        //             std::string msg = reader.readString();
        //             std::cout << "Client: received message=" << msg << std::endl;
        //         }
        //         std::cout << "Client: disconnected" << std::endl;
        //     } catch (SocketException& e) {
        //         std::cout << "Client Recv: failed to connect to ip=" << host << " port=" + port << " reason=" << e.what() << std::endl;
        //     } catch (std::exception& e) {
        //         std::cout << "Client Recv Exception: " << e.what() << std::endl;
        //     }

        //     std::cout << "Client: receive thread stopped" << std::endl;

        //     //dispose();
        // }

        void Client::connect(std::string host, int port) {
            // not if already started
            if (isConnecting || connected()) {
                std::cout << "Client: already connected" << std::endl;
                return;
            }

            client = std::make_shared<TcpClient>();

            isConnecting = true;

            // connect to server (blocking)
            client->connect(host, port);
        }

        void Client::dispose() {
            if (isConnecting || connected()) {
                std::cout << "Client: disposing..." << std::endl;
                // close client
                client->close();

                isConnecting = false;
            }
        }

        void Client::disconnect() {
            std::cout << "Client: disconnecting..." << std::endl;
            dispose();
        }

        bool Client::send(ArraySegment<char>& message) {
            if (!connected()) {
                std::cout << "Client: not connected" << std::endl;
                return false;
            }
            if (message.getSize() > maxMessageSize) {
                std::cout << "Client: message size is too big" << std::endl;
                return false;
            }

            // send data
            client->send(message);
            return true;
        }

        ArraySegment<char> Client::receive(int size) {
            if (!connected()) {
                throw SocketException("Client: not connected");
            }

            // receive data
            return client->receive(size);
        }
    }
}
