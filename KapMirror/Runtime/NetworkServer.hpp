#pragma once

#include "TcpListener.hpp"
#include "ClientConnection.hpp"
#include "MagnificentReceivePipe.hpp"
#include <thread>
#include <mutex>
#include <list>
#include <memory>

namespace KapMirror {
    class NetworkServer {
        private:
        volatile bool running;

        int maxMessageSize;

        std::shared_ptr<TcpListener> listener;
        std::thread listenerThread;

        std::list<std::shared_ptr<ClientConnection>> connectionList;
        std::mutex connectionListMutex;

        MagnificentReceivePipe receivePipe;
        int receiveQueueLimit = 10000;

        int sendQueueLimit = 10000;

        int counter;

        public:
        NetworkServer(int _maxMessageSize = 1024);
        ~NetworkServer();

        void close();

        void start(int port);

        int tick(int processLimit);

        void disconnectClient(int clientId);

        void send(int clientId, std::shared_ptr<ArraySegment<byte>> message);

        private:
        void listen(int port);

        void handleConnection(std::shared_ptr<ClientConnection> connection);
    };
}
