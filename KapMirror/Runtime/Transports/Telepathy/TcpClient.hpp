#pragma once

#include "Socket.hpp"
#include "SocketException.hpp"
#include "KapMirror/Runtime/ArraySegment.hpp"

namespace KapMirror {
namespace Telepathy {
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

        void send(byte* buffer, int size);

        bool receive(int maxMessageSize, byte* buffer, int& size);

        bool isReadable() const;

        bool isWritable() const;

        void setBlocking(bool blocking);
    };
}
}
