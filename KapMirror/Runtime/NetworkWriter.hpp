#pragma once

#include "Array.hpp"
#include "ArraySegment.hpp"
#include <string>
#include <stdexcept>

#define BUFFER_SIZE 1024

namespace KapMirror {
    class NetworkWriter {
        private:
        char *buffer;
        int bufferSize;
        int position;

        public:
        NetworkWriter() {
            buffer = new char[BUFFER_SIZE];
            bufferSize = BUFFER_SIZE;
            position = 0;
        }

        ~NetworkWriter() {
            delete[] buffer;
        }

        /**
         * @brief Reset the position to 0
         */
        inline void reset() {
            position = 0;
        }

        /**
         * @brief Write a value to the buffer
         *
         * @tparam T The type of the value
         * @param value Value
         */
        template <typename T>
        inline void write(T value) {
            size_t size = sizeof(T);

            ensureCapacity(position + size);

            std::copy((char*)&value, (char*)&value + size, buffer + position);
            position += size;
        }

        /**
         * @brief Get the buffer
         * @return Array of bytes
         */
        inline char *toArray() {
            return Array::copyArray(buffer, position);
        }

        /**
         * @brief Get the buffer
         * @return ArraySegment<char>
         */
        inline ArraySegment<char> toArraySegment() {
            return ArraySegment<char>(buffer, position);
        }

        /**
         * @brief Get the size of the buffer
         * @return Size
         */
        inline int size() const {
            return position;
        }

        operator ArraySegment<char>() {
            return toArraySegment();
        }

        private:
        inline void ensureCapacity(int value) {
            if (position > bufferSize) {
                Array::Array::resizeArray(buffer, bufferSize, bufferSize * 2);
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
        void writeBytes(char *array, int offset, int count) {
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
        void writeString(std::string value) {
            write((short)value.length());
            writeBytes((char *)value.c_str(), 0, value.length());
        }
    };
}
