#include "GZipCompression.hpp"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

using namespace KapMirror::Experimental;

std::shared_ptr<KapMirror::ArraySegment<byte>> GZipCompression::compress(std::shared_ptr<KapMirror::ArraySegment<byte>> data) {
    std::vector<char> compressed;
    byte *array = data->toArray();
    for (int i = 0; i < data->getSize(); ++i) {
        compressed.push_back(array[i]);
    }
    boost::iostreams::filtering_ostream os;
    os.push(boost::iostreams::gzip_compressor());
    os.push(boost::iostreams::back_inserter(compressed));
    os.reset();

    auto result = ArraySegment<byte>::createArraySegment(reinterpret_cast<char*>(compressed.data()), compressed.size());
    return result;
}

std::shared_ptr<KapMirror::ArraySegment<byte>> GZipCompression::decompress(std::shared_ptr<KapMirror::ArraySegment<byte>> data) {
    std::vector<char> compressed;
    byte *arrayData = data->toArray();
    int size = data->getSize();

    for (int i = 0; i < size; ++i) {
        compressed.push_back(arrayData[i]);
    }

    std::vector<char> decompressed = std::vector<char>();
    boost::iostreams::filtering_ostream os;
    os.push(boost::iostreams::gzip_decompressor());
    os.push(boost::iostreams::back_inserter(decompressed));
    boost::iostreams::write(os, &compressed[0], compressed.size());

    byte *array = &decompressed[0];
    return ArraySegment<byte>::createArraySegment(array, decompressed.size());
}
