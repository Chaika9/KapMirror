#pragma once

#include "NetworkComponent.hpp"
#include "KapMirror/Runtime/NetworkTime.hpp"
#include "Vectors.hpp"

namespace KapMirror {
    class NetworkTransform : public NetworkComponent {
        private:
        // Set to true if the client should be able to move the object
        bool clientAuthority = false;

        // Send N messages per second
        int sendRate = 30;

        long long lastRefreshTime = 0;
        KapEngine::Tools::Vector3 lastPosition = KapEngine::Tools::Vector3(0, 0, 0);

        public:
        NetworkTransform(std::shared_ptr<KapEngine::GameObject> go);
        ~NetworkTransform() = default;

        /**
         * @brief Set to true if the client should be able to move the object.
        */
        bool isClientWithAuthority() const {
            return networkIdentity->hasAuthority() && clientAuthority;
        }

        /**
         * @brief Set the Client Authority object.
         * Set to true if the client should be able to move the object.
         * (default: false)
         *
         * @param isClientAuthority
         */
        void setClientAuthority(bool _clientAuthority);

        /**
         * @brief Set the Send Rate object.
         * Send N messages per second.
         * (default: 30)
         *
         * @param sendRate
         */
        void setSendRate(int _sendRate);

        void onUpdate();

        private:
        void updateServer();

        void updateClient();
    };
}
