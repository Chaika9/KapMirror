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

            /**
             * @brief Start listening
             * @throws SocketException
             */
            void start();

            /**
             * @brief Stop listening
             */
            void stop();

            /**
             * @brief Accept a new client
             * @return New client
             * @throws SocketException
             */
            std::unique_ptr<TcpClient> acceptTcpClient();
        };
    }
}
