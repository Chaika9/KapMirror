#pragma once

#include "Array.hpp"
#include <string>

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

        inline void reset() {
            position = 0;
        }

        template <typename T>
        inline void write(T value) {
            size_t size = sizeof(T);

            ensureCapacity(position + size);

            std::copy((char*)&value, (char*)&value + size, buffer + position);
            position += size;
        }

        inline char *toArray() {
            char *array = new char[position];
            std::copy(buffer, buffer + position, array);
            return array;
        }

        inline int size() {
            return position;
        }

        private:
        inline void ensureCapacity(size_t value) {
            if (position > bufferSize) {
                Array::Array::resizeArray(buffer, bufferSize, bufferSize * 2);
                bufferSize *= 2;
            }
        }

        public:
        void writeBytes(char *array, size_t offset, size_t count) {
            ensureCapacity(position + count);
            std::copy(array + offset, array + offset + count, buffer + position);
            position += count;
        }

        void writeString(std::string value) {
            write((short)value.length());
            writeBytes((char *)value.c_str(), 0, value.length());
        }
    };
}
