#pragma once

#include "Component.hpp"
#include "NetworkIdentity.hpp"
#include "KapMirror/NetworkClient.hpp"
#include "KapMirror/NetworkServer.hpp"

namespace KapMirror {
    class NetworkComponent : public KapEngine::Component {
      protected:
        NetworkIdentity* networkIdentity = nullptr;

      public:
        NetworkComponent(std::shared_ptr<KapEngine::GameObject> go, const std::string& name);
        ~NetworkComponent() = default;

        std::shared_ptr<NetworkServer> getServer() const;

        std::shared_ptr<NetworkClient> getClient() const;

        void onAwake() override;

        void onDestroy() override;

        /**
         * @brief True if this object is on the server and has been spawned.
         */
        bool isServer() const;

        /**
         * @brief True if this object is on the client and has been spawned by the server.
         */
        bool isClient() const;

        bool isLocal() const;

        /**
         * @brief The unique network Id of this object (unique at runtime).
         */
        unsigned int getNetworkId() const { return networkIdentity->getNetworkId(); }

        /**
         * @brief Like start(), but only called on server.
         */
        virtual void onStartServer() {}

        /**
         * @brief Stop event, only called on server.
         */
        virtual void onStopServer() {}

        /**
         * @brief Like start(), but only called on client.
         */
        virtual void onStartClient() {}

        /**
         * @brief Stop event, only called on client.
         */
        virtual void onStopClient() {}

        /**
         * @brief Serialize all the data from this component into payload.
         */
        virtual void serialize(KapMirror::NetworkWriter& writer) {}

        /**
         * @brief Deserialize all the data from payload into this component.
         */
        virtual void deserialize(KapMirror::NetworkReader& reader) {}
    };
} // namespace KapMirror
