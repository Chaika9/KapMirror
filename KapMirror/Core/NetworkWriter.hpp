#pragma once

#include "Array.hpp"
#include "ArraySegment.hpp"
#include "Platform.hpp"
#include <string>
#include <stdexcept>

#define BUFFER_SIZE 1024

namespace KapMirror {
    class NetworkWriter {
      private:
        byte* buffer;
        int bufferSize;
        int position;

      public:
        NetworkWriter() {
            buffer = new byte[BUFFER_SIZE];
            bufferSize = BUFFER_SIZE;
            position = 0;
        }

        ~NetworkWriter() { delete[] buffer; }

        /**
         * @brief Reset the position to 0
         */
        inline void reset() { position = 0; }

        /**
         * @brief Write a value to the buffer
         *
         * @tparam T The type of the value
         * @param value Value
         */
        template <typename T>
        inline void write(T value) {
            int size = sizeof(T);

            ensureCapacity(position + size);

            std::copy((char*)&value, (char*)&value + size, buffer + position);
            position += size;
        }

        /**
         * @brief Get the buffer
         * @return Array of bytes
         */
        inline byte* toArray() { return Array::copyArray(buffer, position); }

        explicit operator byte*() { return toArray(); }

        /**
         * @brief Get the buffer
         * @return ArraySegment<char>
         */
        inline std::shared_ptr<ArraySegment<byte>> toArraySegment() { return ArraySegment<byte>::createArraySegment(buffer, position); }

        /**
         * @brief Get the size of the buffer
         * @return Size
         */
        inline int size() const { return position; }

      private:
        inline void ensureCapacity(int value) {
            if (position > bufferSize) {
                Array::resizeArray(buffer, bufferSize, bufferSize * 2);
                bufferSize *= 2;
            }
        }

      public:
        /**
         * @brief Write a byte array to the buffer
         *
         * @param array Array
         * @param offset Offset
         * @param count Count
         */
        void writeBytes(byte* array, int offset, int count) {
            if (offset < 0 || count < 0) {
                throw std::invalid_argument("offset or count is less than 0");
            }
            if (count == 0) {
                return;
            }
            ensureCapacity(position + count);
            std::copy(array + offset, array + offset + count, buffer + position);
            position += count;
        }

        /**
         * @brief Write a string to the buffer
         *
         * @param value Value
         */
        void writeString(const std::string& value) {
            write((short)value.length());
            writeBytes((byte*)value.c_str(), 0, (int)value.length());
        }

        void writeArraySegment(const std::shared_ptr<ArraySegment<byte>>& value) { writeBytes(value->toArray(), 0, value->getSize()); }
    };
} // namespace KapMirror
