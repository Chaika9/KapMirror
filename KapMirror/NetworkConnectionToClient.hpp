#pragma once

#include "Runtime/Transport.hpp"
#include "NetworkConnection.hpp"

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

        void send(std::shared_ptr<KapMirror::ArraySegment<byte>> data) override {
            Transport::activeTransport->serverSend(connectionId, data);
        }

        int getConnectionId() const {
            return connectionId;
        }
    };
}
