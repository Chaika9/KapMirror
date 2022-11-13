#include "NetworkStatistics.hpp"
#include "KapMirror/Core/NetworkTime.hpp"

using namespace KapMirror::Experimental;

NetworkStatistics::NetworkStatistics(std::shared_ptr<KapEngine::GameObject> go) : KapEngine::Component(go, "NetworkStatistics") {}

void NetworkStatistics::onStart() {
    onClientDataReceived = [this](Transport&, const std::shared_ptr<ArraySegment<byte>>& data) { onClientReceive(data); };
    onClientDataSent = [this](Transport&, const std::shared_ptr<ArraySegment<byte>>& data) { onClientSend(data); };
    onServerDataReceived = [this](Transport&, int, const std::shared_ptr<ArraySegment<byte>>& data) { onServerReceive(data); };
    onServerDataSent = [this](Transport&, int, const std::shared_ptr<ArraySegment<byte>>& data) { onServerSend(data); };

    if (Transport::activeTransport != nullptr) {
        Transport::activeTransport->onClientDataReceived += onClientDataReceived;
        Transport::activeTransport->onClientDataSent += onClientDataSent;
        Transport::activeTransport->onServerDataReceived += onServerDataReceived;
        Transport::activeTransport->onServerDataSent += onServerDataSent;
    } else {
        KapEngine::Debug::error("NetworkStatistics: no available or active Transport");
    }
}

void NetworkStatistics::onDestroy() {
    if (Transport::activeTransport != nullptr) {
        Transport::activeTransport->onClientDataReceived -= onClientDataReceived;
        Transport::activeTransport->onClientDataSent -= onClientDataSent;
        Transport::activeTransport->onServerDataReceived -= onServerDataReceived;
        Transport::activeTransport->onServerDataSent -= onServerDataSent;
    }
}

void NetworkStatistics::onUpdate() {
    // Calculate results every second
    if (NetworkTime::localTime() >= lastIntervalTime + 1000) {
        lastIntervalTime = NetworkTime::localTime();
        if (Transport::activeTransport != nullptr) {
            updateClient();
            updateServer();
        }
    }
}

void NetworkStatistics::updateClient() {
    clientReceivedPacketsPerSecond = clientIntervalReceivedPackets;
    clientReceivedBytesPerSecond = clientIntervalReceivedBytes;
    clientSentPacketsPerSecond = clientIntervalSentPackets;
    clientSentBytesPerSecond = clientIntervalSentBytes;

    clientIntervalReceivedPackets = 0;
    clientIntervalReceivedBytes = 0;
    clientIntervalSentPackets = 0;
    clientIntervalSentBytes = 0;
}

void NetworkStatistics::updateServer() {
    serverReceivedPacketsPerSecond = serverIntervalReceivedPackets;
    serverReceivedBytesPerSecond = serverIntervalReceivedBytes;
    serverSentPacketsPerSecond = serverIntervalSentPackets;
    serverSentBytesPerSecond = serverIntervalSentBytes;

    serverIntervalReceivedPackets = 0;
    serverIntervalReceivedBytes = 0;
    serverIntervalSentPackets = 0;
    serverIntervalSentBytes = 0;
}

void NetworkStatistics::onClientReceive(const std::shared_ptr<ArraySegment<byte>>& data) {
    clientIntervalReceivedPackets++;
    clientIntervalReceivedBytes += data->getSize();

    clientTotalReceivedPackets++;
    clientReceivedBytesTotal += data->getSize();
}

void NetworkStatistics::onClientSend(const std::shared_ptr<ArraySegment<byte>>& data) {
    clientIntervalSentPackets++;
    clientIntervalSentBytes += data->getSize();

    clientTotalSentPackets++;
    clientSentBytesTotal += data->getSize();
}

void NetworkStatistics::onServerReceive(const std::shared_ptr<ArraySegment<byte>>& data) {
    serverIntervalReceivedPackets++;
    serverIntervalReceivedBytes += data->getSize();

    serverTotalReceivedPackets++;
    serverReceivedBytesTotal += data->getSize();
}

void NetworkStatistics::onServerSend(const std::shared_ptr<ArraySegment<byte>>& data) {
    serverIntervalSentPackets++;
    serverIntervalSentBytes += data->getSize();

    serverTotalSentPackets++;
    serverSentBytesTotal += data->getSize();
}

void NetworkStatistics::reset() {
    clientReceivedPacketsPerSecond = 0;
    clientReceivedBytesPerSecond = 0;
    clientSentPacketsPerSecond = 0;
    clientSentBytesPerSecond = 0;
    clientTotalReceivedPackets = 0;
    clientTotalSentPackets = 0;
    clientReceivedBytesTotal = 0;
    clientSentBytesTotal = 0;

    serverReceivedPacketsPerSecond = 0;
    serverReceivedBytesPerSecond = 0;
    serverSentPacketsPerSecond = 0;
    serverSentBytesPerSecond = 0;
    serverTotalReceivedPackets = 0;
    serverTotalSentPackets = 0;
    serverReceivedBytesTotal = 0;
    serverSentBytesTotal = 0;
}
