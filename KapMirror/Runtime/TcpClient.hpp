#pragma once

#include "Socket.hpp"
#include "SocketException.hpp"
#include "KapMirror/Runtime/ArraySegment.hpp"

namespace KapMirror {
    class TcpClient {
        private:
        std::shared_ptr<Socket> socket;
        bool isConnected;

        public:
        TcpClient(std::shared_ptr<Address> address);
        TcpClient(std::shared_ptr<Socket> socket);
        ~TcpClient();

        /**
         * @brief Close the connection
         */
        void close();

        /**
         * @brief Connect to a server
         * @param host Hostname or IP address
         * @param port Port number
         * @throws SocketException
         */
        void connect();

        /**
         * @brief Is the client connected to a server
         * @return True if connected
         */
        bool connected() const {
            return isConnected;
        }

        /**
         * @brief Send data to the server
         * @param data Data to send
         * @throws SocketException
         */
        void send(ArraySegment<byte>& message);

        bool receive(int maxMessageSize, byte* buffer, int& size);

        bool isReadable() const;

        bool isWritable() const;
    };
}
