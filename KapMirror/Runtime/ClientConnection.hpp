#pragma once

#include "TcpClient.hpp"
#include <thread>

namespace KapMirror {
    struct ClientConnection {
        std::shared_ptr<TcpClient> client;
        std::thread thread;
    };
}
