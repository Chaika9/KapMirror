#pragma once

#include "TcpClient.hpp"
#include "MagnificentSendPipe.hpp"
#include <thread>

namespace KapMirror {
    struct ClientConnection {
        int id;
        std::shared_ptr<TcpClient> client;
        std::thread thread;
        MagnificentSendPipe sendPipe;
    };
}
