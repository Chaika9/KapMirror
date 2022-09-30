#pragma once

#include "NetworkConnection.hpp"
#include "Runtime/Transport.hpp"

namespace KapMirror {
    class NetworkConnectionToClient : public NetworkConnection {
        private:
        int connectionId;

        public:
        NetworkConnectionToClient(int _connectionId) : connectionId(_connectionId) {}
        ~NetworkConnectionToClient() = default;

        void disconnect() override {
            Transport::activeTransport->serverDisconnect(connectionId);
        }

        void sendToTransport(std::shared_ptr<ArraySegment<byte>> data) override {
            Transport::activeTransport->serverSend(connectionId, data);
        }

        int getConnectionId() const {
            return connectionId;
        }
    };
}