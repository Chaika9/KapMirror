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

        KapEngine::Dictionary<int, std::shared_ptr<NetworkConnectionToClient>> connections;
        KapEngine::Dictionary<ushort, std::shared_ptr<std::function<void(std::shared_ptr<NetworkConnectionToClient>, NetworkReader&)>>> handlers;
        KapEngine::Dictionary<unsigned int, std::shared_ptr<KapEngine::GameObject>> networkObjects;

        public:
        NetworkServer(NetworkManager& _manager, KapEngine::KEngine& _engine);
        ~NetworkServer() = default;

        bool isActivated() const {
            return active;
        }

        void listen(int maxConnections, int port);

        void shutdown();

        void networkEarlyUpdate();

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

        void disconnectAll() {
            for (auto const& [id, conn] : connections) {
                conn->disconnect();
                onTransportDisconnect(conn->getConnectionId());
            }

            // cleanup
            connections.clear();
        }

        template<typename T, typename = std::enable_if<std::is_base_of<NetworkMessage, T>::value>>
        void registerHandler(std::function<void(std::shared_ptr<NetworkConnectionToClient>, T&)> handler) {
            ushort id = MessagePacking::getId<T>();
            auto messageHandler = [handler](std::shared_ptr<NetworkConnectionToClient> connection, NetworkReader& reader) {
                T message;
                message.deserialize(reader);
                handler(connection, message);
            };
            handlers[id] = std::make_shared<std::function<void(std::shared_ptr<NetworkConnectionToClient>, NetworkReader&)>>(messageHandler);
        }

        template<typename T, typename = std::enable_if<std::is_base_of<NetworkMessage, T>::value>>
        void unregisterHandler(T& message) {
            ushort id = MessagePacking::getId<T>();
            handlers.remove(id);
        }

        void clearHandlers() {
            handlers.clear();
        }

        // KapEngine

        void spawnObject(std::string prefabName, KapEngine::SceneManagement::Scene &scene,
            KapEngine::Tools::Vector3 position,
            std::function<void(std::shared_ptr<KapEngine::GameObject>&)> playload,
            std::shared_ptr<KapEngine::GameObject>& gameObject);

        void spawnObject(std::string prefabName, std::size_t sceneId, KapEngine::Tools::Vector3 position,
            std::function<void(std::shared_ptr<KapEngine::GameObject>&)> playload,
            std::shared_ptr<KapEngine::GameObject>& gameObject) {
            auto& scene = engine.getSceneManager()->getScene(sceneId);
            spawnObject(prefabName, scene, position, playload, gameObject);
        }

        void spawnObject(std::string prefabName, KapEngine::Tools::Vector3 position,
            std::function<void(std::shared_ptr<KapEngine::GameObject>&)> playload,
            std::shared_ptr<KapEngine::GameObject>& gameObject) {
            auto& scene = engine.getSceneManager()->getCurrentScene();
            spawnObject(prefabName, scene, position, playload, gameObject);
        }

        void spawnObject(std::string prefabName, KapEngine::Tools::Vector3 position,
            std::shared_ptr<KapEngine::GameObject>& gameObject) {
            auto& scene = engine.getSceneManager()->getCurrentScene();
            spawnObject(prefabName, scene, position, nullptr, gameObject);
        }

        void destroyObject(unsigned int networkId);

        void destroyObject(std::shared_ptr<KapEngine::GameObject> gameObject);

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

        bool findObject(unsigned int networkId, std::shared_ptr<KapEngine::GameObject>& gameObject);

        void sendObject(std::shared_ptr<KapEngine::GameObject> gameObject, std::shared_ptr<NetworkConnectionToClient> connection);

        public:
        std::function<void(std::shared_ptr<NetworkConnection>)> onConnectedEvent;
        std::function<void(std::shared_ptr<NetworkConnection>)> onDisconnectedEvent;
    };
}
