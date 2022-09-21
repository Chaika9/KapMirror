#pragma once

namespace KapMirror {
    namespace Transports {
        class TcpClient {
            private:
            int client_fd;

            public:
            TcpClient(int _client_fd);
            ~TcpClient();

            void close();
        };
    }
}
