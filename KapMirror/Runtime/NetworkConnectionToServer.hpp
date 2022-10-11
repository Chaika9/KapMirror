#pragma once

#include "NetworkConnection.hpp"
#include "Transport.hpp"
#include "Compression.hpp"

namespace KapMirror {
    class NetworkConnectionToServer : public NetworkConnection {
        public:
        NetworkConnectionToServer() {}
        ~NetworkConnectionToServer() = default;

        unsigned int getNetworkId() override {
            return 0;
        }

        void disconnect() override {
            Transport::activeTransport->clientDisconnect();
        }

        void sendToTransport(std::shared_ptr<ArraySegment<byte>> data) override {
            if (Compression::activeCompression != nullptr) {
                data = Compression::activeCompression->compress(data);
            }
            Transport::activeTransport->clientSend(data);
        }
    };
}
