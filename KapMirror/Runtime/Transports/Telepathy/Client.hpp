#pragma once

#include "TcpClient.hpp"
#include "KapMirror/Runtime/ArraySegment.hpp"
#include <memory>
#include <string>

namespace KapMirror {
    namespace Transports {
        class Client {
            private:
            std::shared_ptr<TcpClient> client;
            int maxMessageSize;
            bool isConnecting = false;

            public:
            Client(int _maxMessageSize = 4096);
            ~Client() = default;

            void connect(std::string host, int port);

            void dispose();

            void disconnect();

            bool connected() const {
                return client.get() != nullptr && client->connected();
            }

            bool connecting() const {
                return isConnecting;
            }

            bool send(ArraySegment<char>& message);

            ArraySegment<char> receive(int size);

            private:
            void receiveThreadFunction(std::string host, int port);
        };
    }
}
