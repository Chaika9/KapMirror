#pragma once

#include "Internal.hpp"

#include <memory>
#include <string>

namespace KapMirror {
    class Address {
        private:
        addrinfo *address = nullptr;

        public:
        Address(std::string host, int port);
        Address(int port, bool passive = true);
        ~Address();

        addrinfo* getAddress() const {
            return address;
        }

        static std::shared_ptr<Address> createAddress(std::string host, int port);

        static std::shared_ptr<Address> createAddress(int port, bool passive = true);
    };
}
