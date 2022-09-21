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

            void listen(int port);
        };
    }
}
