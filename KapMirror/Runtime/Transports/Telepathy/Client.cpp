#include "Client.hpp"
#include "SocketException.hpp"
#include <iostream>

namespace KapMirror {
    namespace Transports {
        Client::Client() {
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

        bool Client::send(ArraySegment<char>& data) {
            if (!connected()) {
                return false;
            }

            // send data
            client->send(data);
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
