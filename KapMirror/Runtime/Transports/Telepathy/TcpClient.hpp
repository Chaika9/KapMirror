#pragma once

#include "KapMirror/Runtime/ArraySegment.hpp"
#include <string>
#include <stdexcept>

namespace KapMirror {
    namespace Transports {
        class TcpClient {
            private:
            int client_fd;
            int target_fd;
            bool isOwner;

            bool isConnected = false;

            public:
            TcpClient();
            TcpClient(int _client_fd, int _server_fd);
            ~TcpClient();

            void connect(std::string host, int port);
            void close();

            bool connected() {
                return isConnected;
            }

            void send(ArraySegment<char>& data);

            ArraySegment<char> receive(int size);
        };
    }
}
