/*
** EPITECH PROJECT, 2022
** RType
** File description:
** Gzip Compression Method
*/

#ifndef RTYPE_GZIPCOMPRESSIONMETHOD_HPP
#define RTYPE_GZIPCOMPRESSIONMETHOD_HPP

#include "ICompressionMethod.hpp"

namespace KapMirror {

    namespace Compression {

        class GzipCompressionMethod : public ICompressionMethod {

        private:
        public:
            GzipCompressionMethod();

            std::shared_ptr<ArraySegment<byte>> compress(std::shared_ptr<ArraySegment<byte>> message);

            std::shared_ptr<ArraySegment<byte>> decompress(byte *data, int size);

        };

    }

}

#endif //RTYPE_GZIPCOMPRESSIONMETHOD_HPP
