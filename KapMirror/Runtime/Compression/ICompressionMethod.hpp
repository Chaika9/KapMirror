#ifndef RTYPE_COMPRESSIONMETHOD_H
#define RTYPE_COMPRESSIONMETHOD_H

#include <memory>

#include "KapMirror/Runtime/Platform.hpp"
#include "KapMirror/Runtime/ArraySegment.hpp"

namespace KapMirror {
    namespace Compression {

        class ICompressionMethod {

        public:
            virtual std::shared_ptr<ArraySegment<byte>> compress(std::shared_ptr<ArraySegment<byte>> message) = 0;
            virtual std::shared_ptr<ArraySegment<byte>> decompress(byte *data, int size) = 0;

        };

    }
}

#endif