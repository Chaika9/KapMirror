#pragma once

#include "ArraySegment.hpp"
#include "Platform.hpp"

namespace KapMirror {
    class Compression {
        public:
        virtual ~Compression() = default;

        virtual std::shared_ptr<ArraySegment<byte>> compress(std::shared_ptr<ArraySegment<byte>> data) = 0;

        virtual std::shared_ptr<ArraySegment<byte>> decompress(std::shared_ptr<ArraySegment<byte>> data) = 0;

        public:
        static std::shared_ptr<Compression> activeCompression;
    };
}
