#pragma once

#include "NetworkConnection.hpp"
#include "Runtime/Transport.hpp"

namespace KapMirror {
    class NetworkConnectionToServer : public NetworkConnection {
        public:
        NetworkConnectionToServer() {}

        void disconnect() override {
            Transport::activeTransport->clientDisconnect();
        }

        void send(std::shared_ptr<KapMirror::ArraySegment<byte>> data) override {
            Transport::activeTransport->clientSend(data);
        }
    };
}
