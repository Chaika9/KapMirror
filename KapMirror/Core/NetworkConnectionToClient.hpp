#pragma once

#include "NetworkConnection.hpp"
#include "Transport.hpp"
#include "Compression.hpp"

namespace KapMirror {
    class NetworkConnectionToClient : public NetworkConnection {
      private:
        int connectionId;

      public:
        explicit NetworkConnectionToClient(int _connectionId) : connectionId(_connectionId) {}

        /**
         * @brief Unique identifier for this connection that is assigned by the transport layer.
         */
        int getConnectionId() override { return connectionId; }

        /**
         * @brief Disconnects this connection.
         */
        void disconnect() override { Transport::activeTransport->serverDisconnect(connectionId); }

        /**
         * @brief Send raw data to the transport.
         * @param data Data to send
         */
        void sendToTransport(const std::shared_ptr<ArraySegment<byte>>& data) override {
            std::shared_ptr<ArraySegment<byte>> dataToSend = data;
            if (Compression::activeCompression != nullptr) {
                dataToSend = Compression::activeCompression->compress(data);
            }
            Transport::activeTransport->serverSend(connectionId, dataToSend);
        }
    };
} // namespace KapMirror
