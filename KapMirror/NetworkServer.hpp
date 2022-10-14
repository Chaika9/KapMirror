#pragma once

#include "Runtime/ArraySegment.hpp"
#include "Runtime/NetworkConnectionToClient.hpp"
#include "Runtime/NetworkMessage.hpp"
#include "Messages.hpp"
#include "KapEngine.hpp"
#include "Platform.hpp"
#include "Dictionary.hpp"
#include "Debug.hpp"
#include <memory>
#include <functional>
#include <iostream>

namespace KapMirror {
    class NetworkManager;

    class NetworkServer {
        private:
        NetworkManager& manager;
        KapEngine::KEngine& engine;

        bool initialized;
        bool active;

        int maxConnections;

        KapEngine::Dictionary<unsigned int, std::shared_ptr<NetworkConnectionToClient>> connections;
        KapEngine::Dictionary<ushort, std::shared_ptr<std::function<void(std::shared_ptr<NetworkConnectionToClient>, NetworkReader&)>>> handlers;
        KapEngine::Dictionary<unsigned int, std::shared_ptr<KapEngine::GameObject>> networkObjects;

        public:
        NetworkServer(NetworkManager& _manager, KapEngine::KEngine& _engine);
        ~NetworkServer() = default;

        bool isActivated() const {
            return active;
        }

        /**
         * @brief Starts server and listens to incoming connections with max connections limit.
        */
        void listen(int maxConnections, int port);

        /**
         * @brief Shuts down the server and disconnects all clients.
        */
        void shutdown();

        /**
         * @brief NetworkEarlyUpdate.
         * (we add this to the KapEngine in OnFixedUpdate)
        */
        void networkEarlyUpdate();

        /**
         * @brief Send a message to all clients which have joined the world (are ready).
        */
        template<typename T, typename = std::enable_if<std::is_base_of<NetworkMessage, T>::value>>
        void sendToAll(T& message) {
            if (!active) {
                KapEngine::Debug::warning("NetworkServer: Cannot send data, server not active");
                return;
            }

            for (auto const& [id, conn] : connections) {
                conn->send(message);
            }
        }

        /**
         * @brief Send a message to a specific client.
        */
        template<typename T, typename = std::enable_if<std::is_base_of<NetworkMessage, T>::value>>
        void sendToClient(T& message, unsigned int networkId) {
            if (!active) {
                KapEngine::Debug::warning("NetworkServer: Cannot send data, server not active");
                return;
            }

            std::shared_ptr<NetworkConnectionToClient> connection;
            if (connections.tryGetValue(networkId, connection)) {
                connection->send(message);
            } else {
                KapEngine::Debug::warning("NetworkServer: Cannot send data, client not found");
            }
        }

        /**
         * @brief Disconnect all connections.
        */
        void disconnectAll() {
            for (auto const& [id, conn] : connections) {
                conn->disconnect();
                onTransportDisconnect(conn->getConnectionId());
            }

            // cleanup
            connections.clear();
        }

        /**
         * @brief Register a handler for a message type T.
        */
        template<typename T, typename = std::enable_if<std::is_base_of<NetworkMessage, T>::value>>
        void registerHandler(std::function<void(std::shared_ptr<NetworkConnectionToClient>, T&)> handler) {
            ushort msgType = MessagePacking::getId<T>();
            if (handlers.containsKey(msgType)) {
                KapEngine::Debug::warning("NetworkServer.registerHandler replacing handler for {" + std::string(typeid(T).name()) + "}, id={" + std::to_string(msgType) + "}. If replacement is intentional, use replaceHandler instead to avoid this warning.");
            }

            auto messageHandler = [handler](std::shared_ptr<NetworkConnectionToClient> connection, NetworkReader& reader) {
                T message;
                message.deserialize(reader);
                handler(connection, message);
            };
            handlers[msgType] = std::make_shared<std::function<void(std::shared_ptr<NetworkConnectionToClient>, NetworkReader&)>>(messageHandler);
        }

        /**
         * @brief Unregister a message handler of type T.
        */
        template<typename T, typename = std::enable_if<std::is_base_of<NetworkMessage, T>::value>>
        void unregisterHandler(T& message) {
            ushort id = MessagePacking::getId<T>();
            handlers.remove(id);
        }

        #pragma region KapEngine

        /**
         * @brief Spawn new GameObject on specifics Scene for all clients. (only Prefab)
        */
        void spawnObject(std::string prefabName, KapEngine::SceneManagement::Scene &scene,
            KapEngine::Tools::Vector3 position,
            std::function<void(std::shared_ptr<KapEngine::GameObject>&)> playload,
            std::shared_ptr<KapEngine::GameObject>& gameObject);

        /**
         * @brief Spawn new GameObject on specifics Scene for all clients. (only Prefab)
        */
        void spawnObject(std::string prefabName, std::size_t sceneId, KapEngine::Tools::Vector3 position,
            std::function<void(std::shared_ptr<KapEngine::GameObject>&)> playload,
            std::shared_ptr<KapEngine::GameObject>& gameObject) {
            auto& scene = engine.getSceneManager()->getScene(sceneId);
            spawnObject(prefabName, scene, position, playload, gameObject);
        }

        /**
         * @brief Spawn new GameObject on current Scene for all clients. (only Prefab)
        */
        void spawnObject(std::string prefabName, KapEngine::Tools::Vector3 position,
            std::function<void(std::shared_ptr<KapEngine::GameObject>&)> playload,
            std::shared_ptr<KapEngine::GameObject>& gameObject) {
            auto& scene = engine.getSceneManager()->getCurrentScene();
            spawnObject(prefabName, scene, position, playload, gameObject);
        }

        /**
         * @brief Spawn new GameObject for all clients. (only Prefab)
        */
        void spawnObject(std::string prefabName, KapEngine::Tools::Vector3 position,
            std::shared_ptr<KapEngine::GameObject>& gameObject) {
            auto& scene = engine.getSceneManager()->getCurrentScene();
            spawnObject(prefabName, scene, position, nullptr, gameObject);
        }

        /**
         * @brief This takes an object that has been spawned and un-spawns it.
         * The object will be removed from clients.
        */
        void unSpawn(std::shared_ptr<KapEngine::GameObject> gameObject);

        /**
         * @brief Destroy GameObject corresponding of networkId for all clients.
        */
        void destroyObject(unsigned int networkId);

        /**
         * @brief Destroy GameObject for all clients.
        */
        void destroyObject(std::shared_ptr<KapEngine::GameObject> gameObject) {
            unSpawn(gameObject);
        }

        /**
         * @brief Get existing GameObject with networkId.
        */
        bool getExistingObject(unsigned int id, std::shared_ptr<KapEngine::GameObject>& gameObject) {
            return networkObjects.tryGetValue(id, gameObject);
        }

        #pragma endregion

        private:
        void initialize();

        void addTransportHandlers();
        void removeTransportHandlers();

        void onTransportConnect(int connectionId);
        void onTransportDisconnect(int connectionId);
        void onTransportData(int connectionId, std::shared_ptr<ArraySegment<byte>> data);

        bool addConnection(std::shared_ptr<NetworkConnectionToClient> connection);
        bool removeConnection(int connectionId);

        bool unpackAndInvoke(std::shared_ptr<NetworkConnectionToClient> connection, std::shared_ptr<ArraySegment<byte>> data);

        void sendObject(std::shared_ptr<KapEngine::GameObject> gameObject, std::shared_ptr<NetworkConnectionToClient> connection);

        public:
        std::function<void(std::shared_ptr<NetworkConnection>)> onConnectedEvent;
        std::function<void(std::shared_ptr<NetworkConnection>)> onDisconnectedEvent;
    };
}
