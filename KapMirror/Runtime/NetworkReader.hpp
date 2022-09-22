#pragma once

#include "Array.hpp"
#include "ArraySegment.hpp"
#include "Platform.hpp"
#include <string>

namespace KapMirror {
    class NetworkReader {
        private:
        byte *buffer;
        int position;

        public:
        NetworkReader(byte *_buffer) : buffer(_buffer) {
            position = 0;
        }

        NetworkReader(ArraySegment<byte>& segment) {
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
        char *readBytes(int count) {
            byte* array = new byte[count];
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
