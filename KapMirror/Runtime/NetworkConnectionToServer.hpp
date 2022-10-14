#pragma once

#include "NetworkConnection.hpp"
#include "Transport.hpp"
#include "Compression.hpp"

namespace KapMirror {
    class NetworkConnectionToServer : public NetworkConnection {
        public:
        NetworkConnectionToServer() {}
        ~NetworkConnectionToServer() = default;

        /**
         * @brief Unique identifier for this connection that is assigned by the transport layer.
         * (always 0 for server)
        */
        unsigned int getConnectionId() override {
            return 0;
        }

        /**
         * @brief Disconnects from the server.
        */
        void disconnect() override {
            Transport::activeTransport->clientDisconnect();
        }

        /**
         * @brief Send raw data to the transport.
         *
         * @param data Data to send
         */
        void sendToTransport(std::shared_ptr<ArraySegment<byte>> data) override {
            if (Compression::activeCompression != nullptr) {
                data = Compression::activeCompression->compress(data);
            }
            Transport::activeTransport->clientSend(data);
        }
    };
}
