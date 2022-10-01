#pragma once

#include "Runtime/ArraySegment.hpp"
#include "Runtime/Platform.hpp"
#include "Runtime/Dictionary.hpp"
#include "Runtime/NetworkMessage.hpp"
#include "Runtime/NetworkConnectionToServer.hpp"
#include "KapEngine.hpp"
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

    class NetworkClient {
        ConnectState connectState;
        std::shared_ptr<NetworkConnectionToServer> connection;

        Dictionary<ushort, std::shared_ptr<std::function<void(std::shared_ptr<NetworkConnectionToServer>, NetworkReader&)>>> handlers;

        public:
        NetworkClient();
        ~NetworkClient() = default;

        void connect(std::string ip, int port);

        void disconnect();

        void networkEarlyUpdate();

        bool active() const {
            return connectState == ConnectState::Connecting || connectState == ConnectState::Connected;
        }

        bool isConnecting() const {
            return connectState == ConnectState::Connecting;
        }

        bool isConnected() const {
            return connectState == ConnectState::Connected;
        }

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

        template<typename T, typename = std::enable_if<std::is_base_of<NetworkMessage, T>::value>>
        void registerHandler(std::function<void(std::shared_ptr<NetworkConnectionToServer>, T&)> handler) {
            ushort id = MessagePacking::getId<T>();
            auto messageHandler = [handler](std::shared_ptr<NetworkConnectionToServer> connection, NetworkReader& reader) {
                T message;
                message.deserialize(reader);
                handler(connection, message);
            };
            handlers[id] = std::make_shared<std::function<void(std::shared_ptr<NetworkConnectionToServer>, NetworkReader&)>>(messageHandler);
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
        void addTransportHandlers();
        void removeTransportHandlers();

        void onTransportConnect();
        void onTransportDisconnect();
        void onTransportData(std::shared_ptr<ArraySegment<byte>> data);

        bool unpackAndInvoke(std::shared_ptr<ArraySegment<byte>> data);

        public:
        std::function<void(std::shared_ptr<NetworkConnection>)> onConnectedEvent;
        std::function<void(std::shared_ptr<NetworkConnection>)> onDisconnectedEvent;
    };
}
