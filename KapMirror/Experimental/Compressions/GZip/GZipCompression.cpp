#include "GZipCompression.hpp"
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <vector>

using namespace KapMirror::Experimental;

std::shared_ptr<KapMirror::ArraySegment<byte>> GZipCompression::compress(std::shared_ptr<KapMirror::ArraySegment<byte>> data) {
    std::vector<char> compressed;
    boost::iostreams::filtering_ostream os;
    os.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip::best_compression));
    os.push(boost::iostreams::back_inserter(compressed));
    os.write((char*)data->toArray(), data->getSize());
    os.reset();
    return ArraySegment<byte>::createArraySegment(reinterpret_cast<byte*>(compressed.data()), compressed.size());
}

std::shared_ptr<KapMirror::ArraySegment<byte>> GZipCompression::decompress(std::shared_ptr<KapMirror::ArraySegment<byte>> data) {
    std::vector<char> decompressed;
    boost::iostreams::filtering_ostream os;
    os.push(boost::iostreams::gzip_decompressor());
    os.push(boost::iostreams::back_inserter(decompressed));
    os.write((char*)data->toArray(), data->getSize() - 1);
    os.reset();
    return ArraySegment<byte>::createArraySegment(reinterpret_cast<byte*>(decompressed.data()), decompressed.size());
}
