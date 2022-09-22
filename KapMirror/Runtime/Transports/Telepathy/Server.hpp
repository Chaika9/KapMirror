#pragma once

#include "TcpListener.hpp"
#include <thread>

namespace KapMirror {
    namespace Transports {
        class Server {
            private:
            std::shared_ptr<TcpListener> listener;
            std::thread listenerThread;

            public:
            Server();
            ~Server() = default;

            void start(int port);

            private:
            void listen(int port);
        };
    }
}
