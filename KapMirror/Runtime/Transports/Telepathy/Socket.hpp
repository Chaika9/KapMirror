#pragma once

#include "Internal.hpp"
#include "Address.hpp"
#include "KapMirror/Runtime/ArraySegment.hpp"

#include <memory>
#include <string>

namespace KapMirror {
namespace Telepathy {
    class Socket {
        private:
        std::shared_ptr<Address> address;
        int socket_fd;

        public:
        Socket(std::shared_ptr<Address> _address);
        Socket(std::shared_ptr<Address> _address, SOCKET _socket_fd);
        ~Socket();

        void close();

        void bind();

        void listen();

        void connect();

        std::shared_ptr<Socket> accept();

        void setBlocking(bool blocking);

        void send(byte* buffer, int size, uint32_t flags = 0);

        int receive(byte* buffer, int size, uint32_t flags = 0);

        bool isInvalid() const;

        bool isReadable() const;

        bool isWritable() const;

        static std::shared_ptr<Socket> createSocket(std::shared_ptr<Address> address);

        static std::shared_ptr<Socket> createSocket(std::shared_ptr<Address> address, SOCKET socket_fd);
    };
}
}
