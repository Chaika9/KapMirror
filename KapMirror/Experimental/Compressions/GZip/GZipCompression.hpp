#pragma once

#include "KapMirror/Core/Compression.hpp"

namespace KapMirror::Experimental {
    class GZipCompression : public Compression {
      public:
        std::shared_ptr<ArraySegment<byte>> compress(std::shared_ptr<ArraySegment<byte>> data) override;

        std::shared_ptr<ArraySegment<byte>> decompress(std::shared_ptr<ArraySegment<byte>> data) override;
    };
} // namespace KapMirror::Experimental
