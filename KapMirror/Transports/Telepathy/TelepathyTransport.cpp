#include "TelepathyTransport.hpp"
#include <iostream>

using namespace KapMirror;

TelepathyTransport::~TelepathyTransport() {
    if (server != nullptr) {
        server->close();
    }
}

/**
 * Client
 */

void TelepathyTransport::createClient() {
    client = std::make_shared<Telepathy::Client>(clientMaxMessageSize);
    client->sendQueueLimit = clientSendQueueLimit;
    client->receiveQueueLimit = clientReceiveQueueLimit;

    client->onConnected = [this]() { onClientConnected(*this); };
    client->onDisconnected = [this]() { onClientDisconnected(*this); };
    client->onData = [this](const std::shared_ptr<ArraySegment<byte>>& data) { onClientDataReceived(*this, data); };
}

bool TelepathyTransport::clientConnected() { return client != nullptr && client->connected(); }

void TelepathyTransport::clientConnect(const std::string& ip, int port) {
    createClient();
    client->connect(ip, port);
}

void TelepathyTransport::clientDisconnect() {
    if (client != nullptr) {
        client->disconnect();
    }

    // clean values
    client = nullptr;
}

void TelepathyTransport::clientSend(std::shared_ptr<ArraySegment<byte>> data) {
    if (client != nullptr) {
        client->send(data);

        onClientDataSent(*this, data);
    }
}

void TelepathyTransport::clientEarlyUpdate() {
    if (client != nullptr) {
        client->tick(clientMaxReceivesPerTick);
    }
}

/**
 * Server
 */

void TelepathyTransport::serverStart(int port) {
    // Create server
    server = std::make_shared<Telepathy::Server>(serverMaxMessageSize);
    server->sendQueueLimit = serverSendQueueLimitPerConnection;
    server->receiveQueueLimit = serverReceiveQueueLimitPerConnection;

    // Servers Hooks
    server->onConnected = [this](int connectionId) { onServerConnected(*this, connectionId); };
    server->onDisconnected = [this](int connectionId) { onServerDisconnected(*this, connectionId); };
    server->onData = [this](int connectionId, const std::shared_ptr<ArraySegment<byte>>& data) {
        onServerDataReceived(*this, connectionId, data);
    };

    server->start(port);
}

void TelepathyTransport::serverStop() {
    if (server) {
        server->close();
    }

    // clean values
    server = nullptr;
}

void TelepathyTransport::serverSend(int connectionId, std::shared_ptr<ArraySegment<byte>> data) {
    if (server != nullptr) {
        server->send(connectionId, data);

        onServerDataSent(*this, connectionId, data);
    }
}

void TelepathyTransport::serverDisconnect(int connectionId) {
    if (server != nullptr) {
        server->disconnectClient(connectionId);
    }
}

void TelepathyTransport::serverEarlyUpdate() {
    if (server != nullptr) {
        server->tick(serverMaxReceivesPerTick);
    }
}
