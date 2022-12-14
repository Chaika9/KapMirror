#pragma once

#include "KapMirror/Core/Network/TcpListener.hpp"
#include "KapMirror/Core/ArraySegment.hpp"
#include "MagnificentReceivePipe.hpp"
#include "MagnificentSendPipe.hpp"
#include <thread>
#include <mutex>
#include <list>
#include <memory>
#include <functional>

namespace KapMirror::Telepathy {
    struct ClientConnection {
        unsigned int id;
        std::shared_ptr<TcpClient> client;
        std::thread thread;
        MagnificentSendPipe sendPipe;
    };

    class Server {
      private:
        volatile bool running = false;

        int maxMessageSize;

        std::shared_ptr<TcpListener> listener;
        std::thread listenerThread;

        std::list<std::shared_ptr<ClientConnection>> connectionList;
        std::mutex connectionListMutex;

        MagnificentReceivePipe receivePipe;

        unsigned int counter{};

      public:
        int sendQueueLimit = 10000;
        int receiveQueueLimit = 10000;

      public:
        explicit Server(int _maxMessageSize = 1024);
        ~Server();

        void close();

        void start(int port);

        int tick(int processLimit);

        void disconnectClient(int clientId);

        void send(int clientId, const std::shared_ptr<ArraySegment<byte>>& message);

      private:
        void listen(int port);

        void handleConnection(const std::shared_ptr<ClientConnection>& connection);

      public:
        std::function<void(int)> onConnected;
        std::function<void(int)> onDisconnected;
        std::function<void(int, const std::shared_ptr<ArraySegment<byte>>&)> onData;
    };
} // namespace KapMirror::Telepathy
