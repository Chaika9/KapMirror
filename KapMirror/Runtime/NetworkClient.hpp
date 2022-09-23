#pragma once

#include "TcpClient.hpp"
#include "MagnificentReceivePipe.hpp"
#include "MagnificentSendPipe.hpp"
#include <thread>
#include <mutex>
#include <memory>

namespace KapMirror {
    class NetworkClient {
        private:
        volatile bool running;
        volatile bool isConnecting;

        int maxMessageSize;

        std::shared_ptr<TcpClient> client;
        std::thread clientThread;

        MagnificentReceivePipe receivePipe;
        int receiveQueueLimit = 10000;

        MagnificentSendPipe sendPipe;
        int sendQueueLimit = 10000;

        public:
        NetworkClient(int _maxMessageSize = 1024);
        ~NetworkClient();

        void connect(std::string ip, int port);

        void disconnect();

        bool connecting() const {
            return isConnecting;
        }

        bool connected() const {
            return client != nullptr && client->connected();
        }

        int tick(int processLimit);

        void send(std::shared_ptr<ArraySegment<byte>> message);

        private:
        void dispose();

        void run(std::string ip, int port);
    };
}
