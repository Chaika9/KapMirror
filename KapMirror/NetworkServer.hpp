#pragma once

#include "Runtime/ArraySegment.hpp"
#include "Runtime/Platform.hpp"
#include <memory>
#include <functional>

namespace KapMirror {
    class NetworkServer {
        private:
        bool initialized;

        int maxConnections;

        public:
        NetworkServer();
        ~NetworkServer() = default;

        void listen(int maxConnections);

        void shutdown();

        private:
        void initialize();

        void addTransportHandlers();
        void removeTransportHandlers();

        void onTransportConnect(int connectionId);
        void onTransportDisconnect(int connectionId);
        void onTransportData(int connectionId, std::shared_ptr<ArraySegment<byte>> data);
    };
}
