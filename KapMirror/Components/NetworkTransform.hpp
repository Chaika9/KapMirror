#pragma once

#include "NetworkComponent.hpp"
#include "KapMirror/Runtime/NetworkTime.hpp"
#include "Vectors.hpp"

namespace KapMirror {
    class NetworkTransform : public NetworkComponent {
      private:
        // Set to true if the client should be able to move the object
        bool clientAuthority = false;

        // Active sync of the transform
        bool activeUpdate = true;

        // Active sync of the transform with delay
        bool activeLateUpdate = false;

        // Send N messages per second
        int sendRate = 30;

        // Send late update delay (in seconds)
        int lateUpdateDelay = 1;

        long long lastUpdateRefreshTime = 0;
        long long lastLateUpdateRefreshTime = 0;

        KapEngine::Tools::Vector3 lastPosition = KapEngine::Tools::Vector3(0, 0, 0);

      public:
        explicit NetworkTransform(std::shared_ptr<KapEngine::GameObject> go);
        ~NetworkTransform() = default;

        /**
         * @brief Set to true if the client should be able to move the object.
         */
        bool isClientWithAuthority() const { return networkIdentity->hasAuthority() && clientAuthority; }

        /**
         * @brief Set the Client Authority object.
         * Set to true if the client should be able to move the object.
         * (default: false)
         *
         * @param isClientAuthority
         */
        void setClientAuthority(bool _clientAuthority);

        /**
         * @brief Set the Active Update Transform.
         * Set to true if the transform should be updated every frame.
         * (default: true)
         *
         * @param _activeUpdate
         */
        void setActiveUpdate(bool _activeUpdate);

        /**
         * @brief Set the Active Late Update Transform.
         * Set to true if the transform should be updated with a delay.
         * (default: false)
         *
         * @param _activeLateUpdate
         */
        void setActiveLateUpdate(bool _activeLateUpdate);

        /**
         * @brief Set the Send Rate object.
         * Send N messages per second.
         * (default: 30)
         *
         * @param sendRate The number of messages per second.
         */
        void setSendRate(int rate);

        /**
         * @brief Set the Late Update Delay object.
         * Send late update delay (in seconds).
         * (default: 1)
         *
         * @param lateUpdateDelay The delay in seconds.
         */
        void setLateUpdateDelay(int delay);

        void onUpdate() override;

      private:
        void updateServer();

        void updateClient();
    };
} // namespace KapMirror
