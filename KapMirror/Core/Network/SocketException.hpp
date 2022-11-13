#pragma once

#include <exception>
#include <string>
#include <utility>

namespace KapMirror {
    class SocketException : public std::exception {
        std::string _message;

      public:
        explicit SocketException(std::string message) : _message(std::move(message)) {}

        char const* what() const noexcept { return _message.c_str(); }
    };
} // namespace KapMirror
