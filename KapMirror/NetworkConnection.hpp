#pragma once

#include "NetworkMessage.hpp"
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
         * @brief Send a NetworkMessage to this connection
         */
        virtual void send(NetworkMessage& message) = 0;
    };
}
