#pragma once

#include "Runtime/ArraySegment.hpp"
#include "Runtime/Platform.hpp"
#include "Runtime/Dictionary.hpp"
#include "NetworkConnectionToClient.hpp"
#include <memory>
#include <functional>

namespace KapMirror {
    class NetworkServer {
        private:
        bool initialized;
        bool active;

        int maxConnections;

        Dictionary<int, std::shared_ptr<NetworkConnectionToClient>> connections;

        public:
        NetworkServer();
        ~NetworkServer() = default;

        void listen(int maxConnections);

        void shutdown();

        void networkEarlyUpdate();

        void sendToAll(std::shared_ptr<KapMirror::ArraySegment<byte>> data);

        void disconnectAll();

        private:
        void initialize();

        void addTransportHandlers();
        void removeTransportHandlers();

        void onTransportConnect(int connectionId);
        void onTransportDisconnect(int connectionId);
        void onTransportData(int connectionId, std::shared_ptr<ArraySegment<byte>> data);

        bool addConnection(std::shared_ptr<NetworkConnectionToClient> connection);
        bool removeConnection(int connectionId);

        public:
        std::function<void(std::shared_ptr<NetworkConnection>)> onConnectedEvent;
        std::function<void(std::shared_ptr<NetworkConnection>)> onDisconnectedEvent;
    };
}
