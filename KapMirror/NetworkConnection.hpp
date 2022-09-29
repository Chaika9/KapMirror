#pragma once

#include "Runtime/ArraySegment.hpp"
#include "Runtime/Platform.hpp"
#include <memory>

namespace KapMirror {
    class NetworkConnection {
        public:
        virtual ~NetworkConnection() = default;

        /**
         * @brief Disconnects this connection
         */
        virtual void disconnect() = 0;

        /**
         * @brief Sends data to the remote host
         * @param data The data to send
         */
        virtual void send(std::shared_ptr<KapMirror::ArraySegment<byte>> data) = 0;
    };
}
