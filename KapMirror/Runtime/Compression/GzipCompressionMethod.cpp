/*
** EPITECH PROJECT, 2022
** RType
** File description:
** TODO: add description
*/

#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include "KapMirror/Runtime/ArraySegment.hpp"
#include "GzipCompressionMethod.hpp"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

namespace io = boost::iostreams;

KapMirror::Compression::GzipCompressionMethod::GzipCompressionMethod() {}

std::shared_ptr<KapMirror::ArraySegment<byte>> KapMirror::Compression::GzipCompressionMethod::compress(std::shared_ptr<KapMirror::ArraySegment<byte>> message) {
    std::vector<char> compressed;
    byte *array = message->toArray();
    for (int i = 0; i < message->getSize(); ++i) {
        compressed.push_back(array[i]);
    }
    boost::iostreams::filtering_ostream os;
    os.push(boost::iostreams::gzip_compressor());
    os.push(boost::iostreams::back_inserter(compressed));
    os.reset();

    auto result = ArraySegment<byte>::createArraySegment(reinterpret_cast<char*>(compressed.data()), compressed.size());
    return result;
}

std::shared_ptr<KapMirror::ArraySegment<byte>> KapMirror::Compression::GzipCompressionMethod::decompress(byte *data, int size) {
    std::vector<char> compressed;
    for (int i = 0; i < size; ++i) {
        compressed.push_back(data[i]);
    }

    auto result = ArraySegment<byte>::createArraySegment(data, size);

    std::vector<char> decompressed = std::vector<char>();
    boost::iostreams::filtering_ostream os;
    os.push(boost::iostreams::gzip_decompressor());
    os.push(boost::iostreams::back_inserter(decompressed));
    boost::iostreams::write(os, &compressed[0], compressed.size());

    byte *array = &decompressed[0];
    return ArraySegment<byte>::createArraySegment(array, decompressed.size());
}
