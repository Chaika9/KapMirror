#pragma once

#include "KapMirror/Components/NetworkComponent.hpp"

namespace KapMirror::Experimental {
    class NetworkStatistics : public KapEngine::Component {
      private:
        long long lastIntervalTime = 0;

      public:
        // Client
        int clientIntervalReceivedPackets = 0;
        long clientIntervalReceivedBytes = 0;
        int clientIntervalSentPackets = 0;
        long clientIntervalSentBytes = 0;

        int clientReceivedPacketsPerSecond = 0;
        long clientReceivedBytesPerSecond = 0;
        int clientSentPacketsPerSecond = 0;
        long clientSentBytesPerSecond = 0;

        // Server
        int serverIntervalReceivedPackets = 0;
        long serverIntervalReceivedBytes = 0;
        int serverIntervalSentPackets = 0;
        long serverIntervalSentBytes = 0;

        int serverReceivedPacketsPerSecond = 0;
        long serverReceivedBytesPerSecond = 0;
        int serverSentPacketsPerSecond = 0;
        long serverSentBytesPerSecond = 0;

      public:
        explicit NetworkStatistics(std::shared_ptr<KapEngine::GameObject> go);
        ~NetworkStatistics() = default;

        void onStart() override;

        void onDestroy() override;

        void onUpdate() override;

      private:
        void onClientReceive(const std::shared_ptr<ArraySegment<byte>>& data);

        void onClientSend(const std::shared_ptr<ArraySegment<byte>>& data);

        void onServerReceive(const std::shared_ptr<ArraySegment<byte>>& data);

        void onServerSend(const std::shared_ptr<ArraySegment<byte>>& data);

        void updateClient();

        void updateServer();
    };
} // namespace KapMirror::Experimental