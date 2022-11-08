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

    client->onConnected = [this]() {
        if (onClientConnected) {
            onClientConnected(*this);
        }
    };
    client->onDisconnected = [this]() {
        if (onClientDisconnected) {
            onClientDisconnected(*this);
        }
    };
    client->onData = [this](std::shared_ptr<ArraySegment<byte>> data) {
        if (onClientDataReceived) {
            onClientDataReceived(*this, data);
        }
    };
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
    server->onConnected = [this](int connectionId) {
        if (onServerConnected) {
            onServerConnected(*this, connectionId);
        }
    };
    server->onDisconnected = [this](int connectionId) {
        if (onServerDisconnected) {
            onServerDisconnected(*this, connectionId);
        }
    };
    server->onData = [this](int connectionId, std::shared_ptr<ArraySegment<byte>> data) {
        if (onServerDataReceived) {
            onServerDataReceived(*this, connectionId, data);
        }
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
