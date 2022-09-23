#pragma once

#include "Component.hpp"
#include "Runtime/Transport.hpp"
#include "NetworkClient.hpp"
#include "NetworkServer.hpp"

namespace KapMirror {
    class NetworkManager : public KapEngine::Component {
        private:
        std::shared_ptr<Transport> transport;
        std::shared_ptr<NetworkServer> server;
        std::shared_ptr<NetworkClient> client;

        public:
        int maxConnections = 100;

        public:
        NetworkManager(std::shared_ptr<KapEngine::GameObject> go);
        ~NetworkManager();

        void onAwake() override;

        void onStart() override;

        void onUpdate() override;

        void setTransport(std::shared_ptr<Transport> transport);

        void startServer();

        void stopServer();

        void startClient();

        void stopClient();

        private:
        void setupServer();
    };
}

