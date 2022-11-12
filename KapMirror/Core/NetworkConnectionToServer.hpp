#pragma once

#include "NetworkConnection.hpp"
#include "Transport.hpp"
#include "Compression.hpp"

namespace KapMirror {
    class NetworkConnectionToServer : public NetworkConnection {
      public:
        /**
         * @brief Unique identifier for this connection that is assigned by the transport layer.
         * (always 0 for server)
         */
        int getConnectionId() override { return 0; }

        /**
         * @brief Disconnects from the server.
         */
        void disconnect() override { Transport::activeTransport->clientDisconnect(); }

        /**
         * @brief Send raw data to the transport.
         * @param data Data to send
         */
        void sendToTransport(const std::shared_ptr<ArraySegment<byte>>& data) override {
            std::shared_ptr<ArraySegment<byte>> dataToSend = data;
            if (Compression::activeCompression != nullptr) {
                dataToSend = Compression::activeCompression->compress(data);
            }
            Transport::activeTransport->clientSend(dataToSend);
        }
    };
} // namespace KapMirror
