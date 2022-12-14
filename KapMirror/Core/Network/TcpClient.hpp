#pragma once

#include "Socket.hpp"
#include "SocketException.hpp"
#include "KapMirror/Core/ArraySegment.hpp"

namespace KapMirror {
    class TcpClient {
      private:
        std::shared_ptr<Socket> socket;
        bool isConnected;

      public:
        explicit TcpClient(const std::shared_ptr<Address>& address);
        explicit TcpClient(const std::shared_ptr<Socket>& socket);
        ~TcpClient();

        /**
         * @brief Close the connection
         */
        void close();

        /**
         * @brief Connect to a server
         *
         * @param host Hostname or IP address
         * @param port Port number
         *
         * @throws SocketException
         */
        void connect();

        /**
         * @brief Is the client connected to a server
         *
         * @return True if connected
         */
        bool connected() const { return isConnected; }

        /**
         * @brief Send data to the server
         *
         * @param buffer Buffer to send
         * @param size Size of the buffer
         */
        void send(byte* buffer, int size);

        /**
         * @brief Receive data from the server
         *
         * @param buffer Buffer to receive into
         * @param size Size of the buffer
         *
         * @return Connection is closed
         */
        bool receive(int maxMessageSize, byte* buffer, int& size);

        bool isReadable() const;

        bool isWritable() const;

        void setBlocking(bool blocking);
    };
} // namespace KapMirror
