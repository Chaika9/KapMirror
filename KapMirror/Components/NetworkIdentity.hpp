#pragma once

#include "Component.hpp"

namespace KapMirror {
    class NetworkIdentity : public KapEngine::Component {
        private:
        static int nextNetworkId;

        private:
        unsigned int _networkId;
        bool _hasAuthority;

        bool _isServer;
        bool _isClient;

        public:
        NetworkIdentity(std::shared_ptr<KapEngine::GameObject> go);
        ~NetworkIdentity() = default;

        /**
         * @brief The unique network Id of this object (unique at runtime).
        */
        unsigned int getNetworkId() const {
            return _networkId;
        }

        void setNetworkId(unsigned int networkId) {
            _networkId = networkId;
        }

        /**
         * @brief True on client if that component has been assigned to the client.
        */
        bool hasAuthority() const {
            return _hasAuthority;
        }

        void setAuthority(bool authority) {
            _hasAuthority = authority;
        }

        /**
         * @brief Returns true if NetworkServer.isActive and server is not stopped.
        */
        bool isServer() const {
            return _isServer;
        }

        /**
         * @brief Returns true if running as a client and this object was spawned by a server.
        */
        bool isClient() const {
            return _isClient;
        }

        bool isLocal() const {
            return !_isServer && !_isClient;
        }

        void onStartServer();
        void onStopServer();

        void onStartClient();
        void onStopClient();
    };
}
