#pragma once

#include "Runtime/ArraySegment.hpp"
#include "Runtime/Platform.hpp"
#include "NetworkConnectionToClient.hpp"
#include <memory>
#include <functional>
#include <list>

namespace KapMirror {
    class NetworkServer {
        private:
        bool initialized;
        bool active;

        int maxConnections;

        std::list<std::shared_ptr<NetworkConnectionToClient>> connections;

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
        bool tryGetConnection(int connectionId, std::shared_ptr<NetworkConnectionToClient>& connection);
        bool connectionExists(int connectionId);
    };
}
