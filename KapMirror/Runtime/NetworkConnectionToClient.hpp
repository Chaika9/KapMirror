#pragma once

#include "NetworkConnection.hpp"
#include "Transport.hpp"
#include "Compression.hpp"

namespace KapMirror {
    class NetworkConnectionToClient : public NetworkConnection {
        private:
        unsigned int connectionId;

        public:
        NetworkConnectionToClient(unsigned int _connectionId) : connectionId(_connectionId) {}
        ~NetworkConnectionToClient() = default;

        unsigned int getNetworkId() override {
            return connectionId;
        }

        void disconnect() override {
            Transport::activeTransport->serverDisconnect(connectionId);
        }

        void sendToTransport(std::shared_ptr<ArraySegment<byte>> data) override {
            if (Compression::activeCompression != nullptr) {
                data = Compression::activeCompression->compress(data);
            }
            Transport::activeTransport->serverSend(connectionId, data);
        }

        int getConnectionId() const {
            return connectionId;
        }
    };
}
