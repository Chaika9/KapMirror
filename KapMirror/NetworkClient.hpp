#pragma once

#include "Runtime/ArraySegment.hpp"
#include "Runtime/NetworkConnectionToServer.hpp"
#include "Runtime/NetworkMessage.hpp"
#include "Messages.hpp"
#include "KapEngine.hpp"
#include "Platform.hpp"
#include "Dictionary.hpp"
#include "Debug.hpp"
#include <memory>
#include <functional>

namespace KapMirror {
    enum ConnectState {
        None,
        // connecting between Connect() and OnTransportConnected()
        Connecting,
        Connected,
        // disconnecting between Disconnect() and OnTransportDisconnected()
        Disconnecting,
        Disconnected
    };

    class NetworkManager;

    class NetworkClient {
        private:
        NetworkManager& manager;
        KapEngine::KEngine& engine;

        ConnectState connectState;
        std::shared_ptr<NetworkConnectionToServer> connection;

        KapEngine::Dictionary<ushort, std::shared_ptr<std::function<void(std::shared_ptr<NetworkConnectionToServer>, NetworkReader&)>>> handlers;
        KapEngine::Dictionary<unsigned int, std::shared_ptr<KapEngine::GameObject>> networkObjects;

        public:
        NetworkClient(NetworkManager& _manager, KapEngine::KEngine& _engine);
        ~NetworkClient() = default;

        /**
         * @brief Connect client to a NetworkServer by ip and port.
        */
        void connect(std::string ip, int port);

        /**
         * @brief Disconnect from server.
        */
        void disconnect();

        /**
         * @brief NetworkEarlyUpdate.
         * (we add this to the KapEngine in OnFixedUpdate)
        */
        void networkEarlyUpdate();

        /**
         * @brief isActive is true while a client is isConnecting/isConnected.
         * (= while the network is active)
        */
        bool isActive() const {
            return connectState == ConnectState::Connecting || connectState == ConnectState::Connected;
        }

        /**
         * @brief Check if client is connecting (before isConnected).
        */
        bool isConnecting() const {
            return connectState == ConnectState::Connecting;
        }

        /**
         * @brief Check if client is connected (after isConnecting).
        */
        bool isConnected() const {
            return connectState == ConnectState::Connected;
        }

        /**
         * @brief Send a NetworkMessage to the server.
        */
        template<typename T, typename = std::enable_if<std::is_base_of<NetworkMessage, T>::value>>
        void send(T& message) {
            if (connection != nullptr) {
                if (connectState == ConnectState::Connected) {
                    connection->send(message);
                } else {
                    KapEngine::Debug::error("NetworkClient: Send when not connected to a server");
                }
            } else {
                KapEngine::Debug::error("NetworkClient: Send with no connection");
            }
        }

        /**
         * @brief Register a handler for a message type T.
        */
        template<typename T, typename = std::enable_if<std::is_base_of<NetworkMessage, T>::value>>
        void registerHandler(std::function<void(std::shared_ptr<NetworkConnectionToServer>, T&)> handler) {
            ushort msgType = MessagePacking::getId<T>();
            if (handlers.containsKey(msgType)) {
                KapEngine::Debug::warning("NetworkClient.registerHandler replacing handler for {" + std::string(typeid(T).name()) + "}, id={" + std::to_string(msgType) + "}. If replacement is intentional, use replaceHandler instead to avoid this warning.");
            }

            auto messageHandler = [handler](std::shared_ptr<NetworkConnectionToServer> connection, NetworkReader& reader) {
                T message;
                message.deserialize(reader);
                handler(connection, message);
            };
            handlers[msgType] = std::make_shared<std::function<void(std::shared_ptr<NetworkConnectionToServer>, NetworkReader&)>>(messageHandler);
        }

        /**
         * @brief Unregister a message handler of type T.
        */
        template<typename T, typename = std::enable_if<std::is_base_of<NetworkMessage, T>::value>>
        void unregisterHandler(T& message) {
            ushort msgType = MessagePacking::getId<T>();
            handlers.remove(msgType);
        }

        #pragma region KapEngine

        /**
         * @brief Get existing GameObject with networkId.
         * (Spawned by server)
        */
        bool getExistingObject(unsigned int networkId, std::shared_ptr<KapEngine::GameObject>& gameObject) {
            return networkObjects.tryGetValue(networkId, gameObject);
        }

        #pragma endregion

        private:
        void addTransportHandlers();
        void removeTransportHandlers();
        void registerSystemHandlers();

        void onTransportConnect();
        void onTransportDisconnect();
        void onTransportData(std::shared_ptr<ArraySegment<byte>> data);

        bool unpackAndInvoke(std::shared_ptr<ArraySegment<byte>> data);

        // KapEngine
        void onObjectSpawn(ObjectSpawnMessage& message);
        void onObjectDestroy(ObjectDestroyMessage& message);
        void onObjectTransformUpdate(ObjectTransformMessage& message);

        public:
        std::function<void(std::shared_ptr<NetworkConnection>)> onConnectedEvent;
        std::function<void(std::shared_ptr<NetworkConnection>)> onDisconnectedEvent;
    };
}
