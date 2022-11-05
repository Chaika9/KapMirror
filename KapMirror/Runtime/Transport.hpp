#pragma once

#include "ArraySegment.hpp"
#include "Platform.hpp"
#include <functional>
#include <memory>

namespace KapMirror {
    class Transport {
      public:
        virtual ~Transport() = default;

        virtual bool clientConnected() = 0;

        virtual void clientConnect(std::string ip, int port) = 0;

        virtual void clientDisconnect() = 0;

        virtual void clientSend(std::shared_ptr<ArraySegment<byte>> data) = 0;

        virtual void clientEarlyUpdate() = 0;

        virtual void serverStart(int port) = 0;

        virtual void serverStop() = 0;

        virtual void serverSend(int connectionId, std::shared_ptr<ArraySegment<byte>> data) = 0;

        virtual void serverDisconnect(int connectionId) = 0;

        virtual void serverEarlyUpdate() = 0;

      public:
        static std::shared_ptr<Transport> activeTransport;

        std::function<void(Transport&)> onClientConnected;
        std::function<void(Transport&)> onClientDisconnected;
        std::function<void(Transport&, std::shared_ptr<ArraySegment<byte>>)> onClientDataReceived;

        std::function<void(Transport&, int)> onServerConnected;
        std::function<void(Transport&, int)> onServerDisconnected;
        std::function<void(Transport&, int, std::shared_ptr<ArraySegment<byte>>)> onServerDataReceived;
    };
} // namespace KapMirror