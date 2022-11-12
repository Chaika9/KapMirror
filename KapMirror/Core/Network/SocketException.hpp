#pragma once

#include <exception>
#include <string>

namespace KapMirror {
    class SocketException : public std::exception {
        std::string _message;

      public:
        explicit SocketException(const std::string& message) : _message(message) {}

        char const* what() const noexcept { return _message.c_str(); }
    };
} // namespace KapMirror
