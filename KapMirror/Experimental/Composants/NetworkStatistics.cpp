#include "NetworkStatistics.hpp"
#include "KapMirror/Core/NetworkTime.hpp"

using namespace KapMirror::Experimental;

NetworkStatistics::NetworkStatistics(std::shared_ptr<KapEngine::GameObject> go) : NetworkComponent(go, "NetworkStatistics") {}

void NetworkStatistics::onStart() {
    if (Transport::activeTransport != nullptr) {
        Transport::activeTransport->onClientDataReceived +=
            [this](Transport&, const std::shared_ptr<ArraySegment<byte>>& data) { onClientReceive(data); };
        Transport::activeTransport->onClientDataSent +=
            [this](Transport&, const std::shared_ptr<ArraySegment<byte>>& data) { onClientSend(data); };
        Transport::activeTransport->onServerDataReceived +=
            [this](Transport&, int, const std::shared_ptr<ArraySegment<byte>>& data) { onServerReceive(data); };
        Transport::activeTransport->onServerDataSent +=
            [this](Transport&, int, const std::shared_ptr<ArraySegment<byte>>& data) { onServerSend(data); };
    } else {
        KapEngine::Debug::error("NetworkStatistics: no available or active Transport");
    }
}

void NetworkStatistics::onDestroy() {
    // TODO: remove events
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
}

void NetworkStatistics::onClientSend(const std::shared_ptr<ArraySegment<byte>>& data) {
    clientIntervalSentPackets++;
    clientIntervalSentBytes += data->getSize();
}

void NetworkStatistics::onServerReceive(const std::shared_ptr<ArraySegment<byte>>& data) {
    serverIntervalReceivedPackets++;
    serverIntervalReceivedBytes += data->getSize();
}

void NetworkStatistics::onServerSend(const std::shared_ptr<ArraySegment<byte>>& data) {
    serverIntervalSentPackets++;
    serverIntervalSentBytes += data->getSize();
}
