#pragma once

#include <string>

namespace KapMirror {
    namespace Transports {
        class TcpClient {
            private:
            int client_fd;
            int server_fd;

            public:
            TcpClient();
            TcpClient(int _client_fd, int _server_fd);
            ~TcpClient();

            void connect(std::string host, int port);
            void close();
        };
    }
}
