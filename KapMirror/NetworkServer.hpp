#pragma once

#include "Runtime/Compression/ICompressionMethod.hpp"
#include "Runtime/ArraySegment.hpp"
#include "Runtime/Platform.hpp"
#include <memory>
#include <functional>

namespace KapMirror {
    class NetworkServer {
        private:
        std::shared_ptr<Compression::ICompressionMethod> compression;
        bool initialized;

        int maxConnections;

        public:
        NetworkServer(std::shared_ptr<Compression::ICompressionMethod> &compression);
        ~NetworkServer() = default;

        void listen(int maxConnections);

        void shutdown();

        void networkEarlyUpdate();

        private:
        void initialize();

        void addTransportHandlers();
        void removeTransportHandlers();

        void onTransportConnect(int connectionId);
        void onTransportDisconnect(int connectionId);
        void onTransportData(int connectionId, std::shared_ptr<ArraySegment<byte>> data);
    };
}
