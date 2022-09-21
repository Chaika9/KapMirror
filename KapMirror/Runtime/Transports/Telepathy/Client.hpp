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

            public:
            Client();
            ~Client() = default;

            void connect(std::string host, int port);
            void dispose();

            bool connected() {
                return client.get() != nullptr && client->connected();
            }

            bool send(ArraySegment<char>& data);

            ArraySegment<char> receive(int size);
        };
    }
}
