#pragma once

#include "Internal.hpp"
#include <memory>
#include <string>

namespace KapMirror::Telepathy {
    class Address {
      private:
        addrinfo* address = nullptr;

      public:
        Address(const std::string& host, int port);
        explicit Address(int port, bool passive = true);
        ~Address();

        /**
         * @brief Get Address Info
         *
         * @return addrinfo*
         */
        addrinfo* getAddress() const { return address; }

        static std::shared_ptr<Address> createAddress(const std::string& host, int port);

        static std::shared_ptr<Address> createAddress(int port, bool passive = true);
    };
} // namespace KapMirror::Telepathy
