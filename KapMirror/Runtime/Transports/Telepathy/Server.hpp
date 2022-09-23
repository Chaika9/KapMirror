#pragma once

#include "TcpListener.hpp"
#include "MagnificentReceivePipe.hpp"
#include "MagnificentSendPipe.hpp"
#include "KapMirror/Runtime/ArraySegment.hpp"
#include <thread>
#include <mutex>
#include <list>
#include <memory>
#include <functional>

namespace KapMirror {
namespace Telepathy {
    struct ClientConnection {
        int id;
        std::shared_ptr<TcpClient> client;
        std::thread thread;
        MagnificentSendPipe sendPipe;
    };

    class Server {
        private:
        volatile bool running;

        int maxMessageSize;

        std::shared_ptr<TcpListener> listener;
        std::thread listenerThread;

        std::list<std::shared_ptr<ClientConnection>> connectionList;
        std::mutex connectionListMutex;

        MagnificentReceivePipe receivePipe;

        int counter;

        public:
        int sendQueueLimit = 10000;
        int receiveQueueLimit = 10000;

        public:
        Server(int _maxMessageSize = 1024);
        ~Server();

        void close();

        void start(int port);

        int tick(int processLimit);

        void disconnectClient(int clientId);

        void send(int clientId, std::shared_ptr<ArraySegment<byte>> message);

        private:
        void listen(int port);

        void handleConnection(std::shared_ptr<ClientConnection> connection);

        public:
        std::function<void(int)> onConnected;
        std::function<void(int)> onDisconnected;
        std::function<void(int, std::shared_ptr<ArraySegment<byte>>)> onData;
    };
}
}
