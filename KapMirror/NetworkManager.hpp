#pragma once

#include "Runtime/Transport.hpp"
#include "Runtime/Compression.hpp"
#include "Runtime/NetworkMessage.hpp"
#include "KapMirror.hpp"
#include "Component.hpp"

namespace KapMirror {
    class NetworkManager : public KapEngine::Component {
        private:
        std::shared_ptr<Transport> transport;
        std::shared_ptr<Compression> compression;

        std::shared_ptr<NetworkServer> server;
        std::shared_ptr<NetworkClient> client;

        public:
        int maxConnections = 100;

        std::string networkAddress = "127.0.0.1";
        int networkPort = 7777;

        public:
        NetworkManager(std::shared_ptr<KapEngine::GameObject> go);
        ~NetworkManager();

        std::shared_ptr<NetworkServer> getServer() const {
            return server;
        }

        std::shared_ptr<NetworkClient> getClient() const {
            return client;
        }

        void onAwake() override;

        void onUpdate() override;

        void setTransport(std::shared_ptr<Transport> transport);

        void setCompression(std::shared_ptr<Compression> compression);

        void startServer();

        void stopServer();

        void startClient();

        void stopClient();

        void __initGameObject(std::shared_ptr<KapEngine::GameObject> go);

        protected:
        // Events
        virtual void onStartServer() {}
        virtual void onStopServer() {}
        virtual void onServerClientConnected(std::shared_ptr<NetworkConnection> connection) {}
        virtual void onServerClientDisconnected(std::shared_ptr<NetworkConnection> connection) {}

        virtual void onStartClient() {}
        virtual void onStopClient() {}
        virtual void onClientConnected(std::shared_ptr<NetworkConnection> connection) {}
        virtual void onClientDisconnected(std::shared_ptr<NetworkConnection> connection) {}

        private:
        void setupServer();
    };
}

