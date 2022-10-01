#pragma once

#include "NetworkConnection.hpp"
#include "Transport.hpp"

namespace KapMirror {
    class NetworkConnectionToServer : public NetworkConnection {
        public:
        NetworkConnectionToServer() {}

        void disconnect() override {
            Transport::activeTransport->clientDisconnect();
        }

        void sendToTransport(std::shared_ptr<ArraySegment<byte>> data) override {
            Transport::activeTransport->clientSend(data);
        }
    };
}
