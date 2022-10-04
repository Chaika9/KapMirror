#pragma once

#include "Component.hpp"
#include "NetworkIdentity.hpp"
#include "KapMirror/NetworkClient.hpp"
#include "KapMirror/NetworkServer.hpp"

namespace KapMirror {
namespace Experimental {
    class NetworkComponent : public KapEngine::Component {
        private:
        std::shared_ptr<NetworkServer> server;
        std::shared_ptr<NetworkClient> client;

        protected:
        NetworkIdentity* networkIdentity;

        public:
        NetworkComponent(std::shared_ptr<KapEngine::GameObject> go, std::string name);
        ~NetworkComponent() = default;

        std::shared_ptr<NetworkServer> getServer() const {
            if (server == nullptr) {
                throw std::runtime_error("NetworkComponent: Server is not set");
            }
            return server;
        }

        void __setServer(std::shared_ptr<NetworkServer> _server) {
            server = _server;
        }

        std::shared_ptr<NetworkClient> getClient() const {
            if (client == nullptr) {
                throw std::runtime_error("NetworkComponent: Client is not set");
            }
            return client;
        }

        void __setClient(std::shared_ptr<NetworkClient> _client) {
            client = _client;
        }

        void onStart() override;

        bool isServer() const;

        bool isClient() const;

        virtual void onStartServer() {}
        virtual void onStopServer() {}

        virtual void onStartClient() {}
        virtual void onStopClient() {}
    };
}
}
