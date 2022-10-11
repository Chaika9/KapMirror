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

        unsigned int getNetworkId() const {
            return _networkId;
        }

        void setNetworkId(unsigned int networkId) {
            _networkId = networkId;
        }

        bool hasAuthority() const {
            return _hasAuthority;
        }

        void setAuthority(bool authority) {
            _hasAuthority = authority;
        }

        bool isServer() const {
            return _isServer;
        }

        bool isClient() const {
            return _isClient;
        }

        void onStartServer();
        void onStopServer();

        void onStartClient();
        void onStopClient();
    };
}
