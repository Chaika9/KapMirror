#pragma once

#include "KapMirror/Runtime/Compression/ICompressionMethod.hpp"
#include "Socket.hpp"
#include "SocketException.hpp"
#include "KapMirror/Runtime/ArraySegment.hpp"

namespace KapMirror {
namespace Telepathy {
    class TcpClient {
        private:
        std::shared_ptr<Socket> socket;
        std::shared_ptr<Compression::ICompressionMethod> compression;
        bool isConnected;

        public:
        TcpClient(std::shared_ptr<Address> address, std::shared_ptr<Compression::ICompressionMethod> &compression);
        TcpClient(std::shared_ptr<Socket> socket, std::shared_ptr<Compression::ICompressionMethod> &compression);
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
        void send(std::shared_ptr<ArraySegment<byte>> message);

        bool receive(int maxMessageSize, byte* buffer, int& size);

        bool isReadable() const;

        bool isWritable() const;

        void setBlocking(bool blocking);
    };
}
}
