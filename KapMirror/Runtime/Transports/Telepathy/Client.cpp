#include "Client.hpp"

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
    }
}
