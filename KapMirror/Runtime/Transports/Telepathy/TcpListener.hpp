#pragma once

#include "TcpClient.hpp"
#include <memory>

namespace KapMirror {
    namespace Transports {
        class TcpListener {
            private:
            int server_fd;
            int port;

            int isConnected = false;

            public:
            TcpListener(int _port);
            ~TcpListener();

            void start();
            void stop();

            std::unique_ptr<TcpClient> acceptTcpClient();
        };
    }
}
