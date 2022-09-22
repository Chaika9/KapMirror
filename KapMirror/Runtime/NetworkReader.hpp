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
            buffer = segment;
            position = 0;
        }

        ~NetworkReader() = default;

        /**
         * @brief Reset the position to 0
         */
        inline void reset() {
            position = 0;
        }

        /**
         * @brief Read a value from the buffer
         *
         * @tparam T The type of the value
         * @return Value
         */
        template <typename T>
        inline T read() {
            T value = *(T*)(buffer + position);
            position += sizeof(T);
            return value;
        }

        /**
         * @brief Read a byte array from the buffer
         *
         * @return Value
         */
        char *readBytes(size_t count) {
            char *array = new char[count];
            std::copy(buffer + position, buffer + position + count, array);
            position += count;
            return array;
        }

        /**
         * @brief Read a string from the buffer
         *
         * @return Value
         */
        std::string readString() {
            short length = read<short>();
            char *array = readBytes(length);
            std::string str(array, length);
            delete[] array;
            return str;
        }
    };
}
