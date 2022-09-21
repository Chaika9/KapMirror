#pragma once

#include "Array.hpp"
#include "ArraySegment.hpp"
#include <string>

#include <iostream>

namespace KapMirror {
    class NetworkReader {
        private:
        char *buffer;
        int position;

        public:
        NetworkReader(char *_buffer) : buffer(_buffer) {
            position = 0;
        }

        NetworkReader(ArraySegment<char> segment) {
            buffer = segment.toArray();
            position = 0;
        }

        ~NetworkReader() = default;

        inline void reset() {
            position = 0;
        }

        template <typename T>
        inline T read() {
            T value = *(T*)(buffer + position);
            position += sizeof(T);
            return value;
        }

        char *readBytes(size_t count) {
            char *array = new char[count];
            std::copy(buffer + position, buffer + position + count, array);
            position += count;
            return array;
        }

        std::string readString() {
            short length = read<short>();
            char *array = readBytes(length);
            std::string str(array, length);
            delete[] array;
            return str;
        }
    };
}
