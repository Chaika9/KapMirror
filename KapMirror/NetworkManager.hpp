#pragma once

#include "Core/Transport.hpp"
#include "Core/Compression.hpp"
#include "Core/NetworkMessage.hpp"
#include "KapMirror.hpp"
#include "Component.hpp"

namespace KapMirror {
    class NetworkManager : public KapEngine::Component {
      private:
        static NetworkManager* instance;

        std::shared_ptr<Transport> transport;
        std::shared_ptr<Compression> compression;

        std::shared_ptr<NetworkServer> server;
        std::shared_ptr<NetworkClient> client;

      public:
        int maxConnections = 100;

        std::string networkAddress = "127.0.0.1";
        int networkPort = 7777;

      public:
        explicit NetworkManager(std::shared_ptr<KapEngine::GameObject> go);
        ~NetworkManager();

        static NetworkManager* getInstance() { return instance; }

        std::shared_ptr<NetworkServer> getServer() const { return server; }

        std::shared_ptr<NetworkClient> getClient() const { return client; }

        void onAwake() override;

        void onFixedUpdate() override;

        /**
         * @brief Set transport layer.
         */
        void setTransport(const std::shared_ptr<Transport>& _transport);

        void setCompression(const std::shared_ptr<Compression>& _compression);

        /**
         * @brief Starts the server, listening for incoming connections.
         */
        void startServer();

        /**
         * @brief Stops the server from listening and simulating the game.
         */
        void stopServer();

        /**
         * @brief Starts the client, connects it to the server with ip and port.
         */
        void startClient();

        /**
         * @brief Stops and disconnects the client.
         */
        void stopClient();

      protected:
        /**
         * @brief This is invoked when a server is started.
         */
        virtual void onStartServer() {}

        /**
         * @brief This is called when a server is stopped.
         */
        virtual void onStopServer() {}

        /**
         * @brief This is invoked when a client is connected to a server.
         */
        virtual void onServerClientConnected(const std::shared_ptr<NetworkConnection>& connection) {}

        /**
         * @brief This is invoked when a client is disconnected from a server.
         */
        virtual void onServerClientDisconnected(const std::shared_ptr<NetworkConnection>& connection) {}

        /**
         * @brief This is invoked when the client is started.
         */
        virtual void onStartClient() {}

        /**
         * @brief This is called when a client is stopped.
         */
        virtual void onStopClient() {}

        /**
         * @brief This is invoked when the client is connected to a server.
         */
        virtual void onClientConnected(const std::shared_ptr<NetworkConnection>& connection) {}

        /**
         * @brief This is invoked when the client is disconnected from a server.
         */
        virtual void onClientDisconnected(const std::shared_ptr<NetworkConnection>& connection) {}

      private:
        void initializeSingleton();

        void setupServer();

        void removeHandlers();
    };
} // namespace KapMirror
