#include "Client.hpp"
#include "SocketException.hpp"
#include <iostream>

namespace KapMirror {
    namespace Transports {
        Client::Client(int _maxMessageSize) : maxMessageSize(_maxMessageSize) {
        }

        void Client::connect(std::string host, int port) {
            this->client = std::make_shared<TcpClient>();

            // connect to server
            this->client->connect(host, port);
        }

        void Client::dispose() {
            // close client
            client->close();
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
