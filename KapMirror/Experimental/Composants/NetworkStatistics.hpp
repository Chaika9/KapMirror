#pragma once

#include "KapMirror/Components/NetworkComponent.hpp"

namespace KapMirror::Experimental {
    class NetworkStatistics : public KapEngine::Component {
      private:
        long long lastIntervalTime = 0;

      public:
        // ---------------------------------------------------------------------

        // CLIENT (public fields for other components to grab statistics)
        // long bytes to support >2GB
        int clientIntervalReceivedPackets = 0;
        long clientIntervalReceivedBytes = 0;
        int clientIntervalSentPackets = 0;
        long clientIntervalSentBytes = 0;

        // total bytes to support >2GB
        long clientReceivedBytesTotal = 0;
        long clientSentBytesTotal = 0;

        // results from last interval
        // long bytes to support >2GB
        int clientReceivedPacketsPerSecond = 0;
        long clientReceivedBytesPerSecond = 0;
        int clientSentPacketsPerSecond = 0;
        long clientSentBytesPerSecond = 0;

        // ---------------------------------------------------------------------

        // SERVER (public fields for other components to grab statistics)
        // capture interval
        // long bytes to support >2GB
        int serverIntervalReceivedPackets = 0;
        long serverIntervalReceivedBytes = 0;
        int serverIntervalSentPackets = 0;
        long serverIntervalSentBytes = 0;

        // total bytes to support >2GB
        long serverReceivedBytesTotal = 0;
        long serverSentBytesTotal = 0;

        // results from last interval
        // long bytes to support >2GB
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

        /**
         * @brief Reset all statistics
         */
        void reset();

      private:
        void onClientReceive(const std::shared_ptr<ArraySegment<byte>>& data);

        void onClientSend(const std::shared_ptr<ArraySegment<byte>>& data);

        void onServerReceive(const std::shared_ptr<ArraySegment<byte>>& data);

        void onServerSend(const std::shared_ptr<ArraySegment<byte>>& data);

        void updateClient();

        void updateServer();
    };
} // namespace KapMirror::Experimental