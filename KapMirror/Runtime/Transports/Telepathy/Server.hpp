#pragma once

#include "TcpListener.hpp"
#include <memory>

namespace KapMirror {
    namespace Transports {
        class Server {
            private:
            std::shared_ptr<TcpListener> listener;

            public:
            Server();
            ~Server() = default;

            bool start(int port);
            void stop();

            private:
            void listen(int port);
        };
    }
}
