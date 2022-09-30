#pragma once

#include "Runtime/NetworkWriter.hpp"
#include "Runtime/MessagePacking.hpp"
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

        virtual void sendToTransport(std::shared_ptr<ArraySegment<byte>> data) = 0;

        /**
         * @brief Send a NetworkMessage to this connection
         */
        template<typename T, typename = std::enable_if<std::is_base_of<NetworkMessage, T>::value>>
        void send(T& message) {
            NetworkWriter writer;
            MessagePacking::pack(message, writer);
            std::shared_ptr<ArraySegment<byte>> data = writer.toArraySegment();
            sendToTransport(data);
        }
    };
}
