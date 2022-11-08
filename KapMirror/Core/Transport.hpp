#pragma once

#include "ArraySegment.hpp"
#include "Platform.hpp"
#include <functional>
#include <memory>

namespace KapMirror {
    class Transport {
      public:
        virtual ~Transport() = default;

        /**
         * @brief Client is connected to the server
         * @return True if the client is connected to the server
         */
        virtual bool clientConnected() = 0;

        /**
         * @brief Connect the client to the server
         * @param ip The ip of the server
         * @param port The port of the server
         */
        virtual void clientConnect(const std::string& ip, int port) = 0;

        /**
         * @brief Disconnect the client from the server
         */
        virtual void clientDisconnect() = 0;

        /**
         * @brief Send data to the server
         * @param data The data to send
         */
        virtual void clientSend(std::shared_ptr<ArraySegment<byte>> data) = 0;

        /**
         * @brief Client Update (call this function every frame)
         */
        virtual void clientEarlyUpdate() = 0;

        /**
         * @brief Start the server and listen the port
         * @param port The port to listen
         */
        virtual void serverStart(int port) = 0;

        /**
         * @brief Stop the server
         */
        virtual void serverStop() = 0;

        /**
         * @brief Send data to the client
         * @param connectionId The id of the connection
         * @param data The data to send
         */
        virtual void serverSend(int connectionId, std::shared_ptr<ArraySegment<byte>> data) = 0;

        /**
         * @brief Disconnect the connection
         * @param connectionId The id of the connection
         */
        virtual void serverDisconnect(int connectionId) = 0;

        /**
         * @brief Server Update (call this function every frame)
         */
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