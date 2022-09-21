#pragma once

#include "Array.hpp"
#include <stdexcept>

namespace KapMirror {
    template <typename T>
    class ArraySegment {
        private:
        T* array;
        int offset;
        int size;

        public:
        ArraySegment(T* _array, int _size) {
            array = Array::Array::copyArray(_array, _size);
            offset = 0;
            size = _size;
        }

        ArraySegment(T* _array, int _offset, int _size) {
            array = Array::Array::copyArray(_array, _size);
            offset = _offset;
            size = _size;
        }

        ~ArraySegment() {
            delete[] array;
        };

        T *toArray() const {
            return array;
        }

        int getSize() const {
            return size;
        }

        int getOffset() const {
            return offset;
        }

        T &operator[](int index) {
            if (index < 0 || index >= size) {
                throw std::out_of_range("index is out of range");
            }
            return array[offset + index];
        }

        bool operator==(ArraySegment<T> &other) {
            for (int i = 0; i < std::min(size, other.size); i++) {
                if (array[offset + i] != other.array[other.offset + i]) {
                    return false;
                }
            }
            return true;
        }

        bool operator!=(ArraySegment<T> &other) {
            return !(*this == other);
        }

        ArraySegment<T> &operator=(ArraySegment<T> &other) {
            array = other.array;
            offset = other.offset;
            size = other.size;
            return *this;
        }

        operator T*() const {
            return array;
        }
    };
}
