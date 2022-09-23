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
            array = Array::copyArray(_array, _size);
            offset = 0;
            size = _size;
        }

        ArraySegment(T* _array, int _offset, int _size) {
            array = Array::copyArray(_array, _size);
            offset = _offset;
            size = _size;
        }

        ~ArraySegment() {
            delete[] array;
        };

        /**
         * @brief Get the array
         * @return Array
         */
        T *toArray() const {
            return array;
        }

        operator T*() const {
            return array;
        }

        /**
         * @brief Get the offset of the array
         * @return Offset
         */
        int getOffset() const {
            return offset;
        }

        /**
         * @brief Get the size of the array
         * @return Size
         */
        int getSize() const {
            return size;
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

        static std::shared_ptr<ArraySegment<T>> createArraySegment(T* array, int size) {
            return std::make_shared<ArraySegment<T>>(array, size);
        }
    };
}
