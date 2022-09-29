#pragma once

#include "Runtime/ArraySegment.hpp"
#include "Runtime/Platform.hpp"
#include <memory>

namespace KapMirror {
    class NetworkConnection {
        public:
        virtual ~NetworkConnection() = default;

        virtual void disconnect() = 0;
        virtual void send(std::shared_ptr<KapMirror::ArraySegment<byte>> data) = 0;
    };
}
