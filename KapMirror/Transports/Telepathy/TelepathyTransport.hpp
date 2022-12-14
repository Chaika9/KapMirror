#pragma once

#include "KapMirror/Core/Transport.hpp"
#include "Client.hpp"
#include "Server.hpp"

namespace KapMirror {
    class TelepathyTransport : public Transport {
      private:
        std::shared_ptr<Telepathy::Client> client;

        std::shared_ptr<Telepathy::Server> server;

      public:
        int clientMaxMessageSize = 16 * 1024;
        int clientMaxReceivesPerTick = 1000;
        int clientSendQueueLimit = 10000;
        int clientReceiveQueueLimit = 10000;

        int serverMaxMessageSize = 16 * 1024;
        int serverMaxReceivesPerTick = 10000;
        int serverSendQueueLimitPerConnection = 10000;
        int serverReceiveQueueLimitPerConnection = 10000;

      public:
        TelepathyTransport() = default;
        ~TelepathyTransport() override;

        bool clientConnected() override;

        void clientConnect(const std::string& ip, int port) override;

        void clientDisconnect() override;

        void clientSend(const std::shared_ptr<ArraySegment<byte>>& data) override;

        void clientEarlyUpdate() override;

        void serverStart(int port) override;

        void serverStop() override;

        void serverSend(int connectionId, const std::shared_ptr<ArraySegment<byte>>& data) override;

        void serverDisconnect(int connectionId) override;

        void serverEarlyUpdate() override;

      private:
        void createClient();
    };
} // namespace KapMirror
