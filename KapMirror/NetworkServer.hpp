#pragma once

#include "Runtime/ArraySegment.hpp"
#include "Runtime/Platform.hpp"
#include "Runtime/Dictionary.hpp"
#include "NetworkConnectionToClient.hpp"
#include "NetworkMessage.hpp"
#include "KapEngine.hpp"
#include "Debug.hpp"
#include <memory>
#include <functional>
#include <iostream>

namespace KapMirror {
    class NetworkServer {
        private:
        bool initialized;
        bool active;

        int maxConnections;

        Dictionary<int, std::shared_ptr<NetworkConnectionToClient>> connections;
        Dictionary<ushort, std::shared_ptr<std::function<void(std::shared_ptr<NetworkConnectionToClient>, NetworkReader&)>>> handlers;

        public:
        NetworkServer();
        ~NetworkServer() = default;

        void listen(int maxConnections);

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

        public:
        std::function<void(std::shared_ptr<NetworkConnection>)> onConnectedEvent;
        std::function<void(std::shared_ptr<NetworkConnection>)> onDisconnectedEvent;
    };
}
