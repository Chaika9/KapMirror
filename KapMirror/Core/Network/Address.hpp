#pragma once

#include "Internal.hpp"
#include <memory>
#include <string>

namespace KapMirror {
    class Address {
      public:
        enum class SocketType { TCP = SOCK_STREAM, UDP = SOCK_DGRAM };

      private:
        addrinfo* address = nullptr;

      public:
        Address();
        Address(const std::string& host, int port, SocketType type = SocketType::TCP);
        explicit Address(int port, bool passive = true, SocketType type = SocketType::TCP);

        ~Address();

        /**
         * @brief Get Address Info
         * @return addrinfo* The address info
         */
        addrinfo* getAddress() const { return address; }

        /**
         * @brief Get the ip address
         * @return The ip address
         */
        std::string toString() const;

        static std::shared_ptr<Address> createAddress();

        static std::shared_ptr<Address> createAddress(const std::string& host, int port, SocketType type = SocketType::TCP);

        static std::shared_ptr<Address> createAddress(int port, bool passive = true, SocketType type = SocketType::TCP);
    };
} // namespace KapMirror
