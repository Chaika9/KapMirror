#pragma once

#include "Array.hpp"
#include "ArraySegment.hpp"
#include "Platform.hpp"
#include <string>
#include <memory>

namespace KapMirror {
    class NetworkReader {
      private:
        byte* buffer = nullptr;
        int position;

      public:
        explicit NetworkReader(byte* _buffer) : buffer(_buffer) { position = 0; }

        explicit NetworkReader(const ArraySegment<byte>& segment) {
            buffer = segment.toArray();
            position = 0;
        }

        explicit NetworkReader(const std::shared_ptr<ArraySegment<byte>>& segment) {
            buffer = segment->toArray();
            position = 0;
        }

        ~NetworkReader() = default;

        /**
         * @brief Reset the position to 0
         */
        inline void reset() { position = 0; }

        /**
         * @brief Read a value from the buffer
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
         * @return Value
         */
        byte* readBytes(int count) {
            byte* array = new byte[count];
            std::copy(buffer + position, buffer + position + count, array);
            position += count;
            return array;
        }

        /**
         * @brief Read a string from the buffer
         * @return Value
         */
        std::string readString() {
            auto length = read<short>();
            byte* array = readBytes(length);
            std::string str((char*)array, length);
            delete[] array;
            return str;
        }
    };
} // namespace KapMirror
