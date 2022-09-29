#pragma once

#include "NetworkConnection.hpp"
#include "Runtime/Transport.hpp"
#include "Runtime/NetworkWriter.hpp"
#include "MessagePacking.hpp"

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

        void send(NetworkMessage& message) override {
            NetworkWriter writer;
            MessagePacking::pack(message, writer);
            std::shared_ptr<ArraySegment<byte>> data = writer.toArraySegment();
            Transport::activeTransport->serverSend(connectionId, data);
        }

        int getConnectionId() const {
            return connectionId;
        }
    };
}
