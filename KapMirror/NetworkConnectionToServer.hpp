#pragma once

#include "NetworkConnection.hpp"
#include "Runtime/Transport.hpp"
#include "Runtime/NetworkWriter.hpp"
#include "MessagePacking.hpp"

namespace KapMirror {
    class NetworkConnectionToServer : public NetworkConnection {
        public:
        NetworkConnectionToServer() {}

        void disconnect() override {
            Transport::activeTransport->clientDisconnect();
        }

        void send(NetworkMessage& message) override {
            NetworkWriter writer;
            MessagePacking::pack(message, writer);
            std::shared_ptr<ArraySegment<byte>> data = writer.toArraySegment();
            Transport::activeTransport->clientSend(data);
        }
    };
}
