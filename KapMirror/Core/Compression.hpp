#pragma once

#include "ArraySegment.hpp"
#include "Platform.hpp"

namespace KapMirror {
    class Compression {
      public:
        virtual ~Compression() = default;

        /**
         * @brief Compress data with the compression algorithm
         * @return Compressed data as ArraySegment<byte>
         */
        virtual std::shared_ptr<ArraySegment<byte>> compress(const std::shared_ptr<ArraySegment<byte>>& data) = 0;

        /**
         * @brief Decompress data with the compression algorithm
         * @return Decompressed data as ArraySegment<byte>
         */
        virtual std::shared_ptr<ArraySegment<byte>> decompress(const std::shared_ptr<ArraySegment<byte>>& data) = 0;

      public:
        static std::shared_ptr<Compression> activeCompression;
    };
} // namespace KapMirror
