#pragma once

#include "TcpListener.hpp"
#include "ClientConnection.hpp"
#include <thread>
#include <mutex>
#include <list>

namespace KapMirror {
    class NetworkServer {
        private:
        volatile bool running;

        std::shared_ptr<TcpListener> listener;
        std::thread listenerThread;

        std::list<std::shared_ptr<ClientConnection>> connectionList;
        std::mutex connectionListMutex;

        public:
        NetworkServer();
        ~NetworkServer();

        void close();

        void start(int port);

        private:
        void listen(int port);

        void handleConnection(std::shared_ptr<ClientConnection> connection);
    };
}
