#pragma once

#include "Array.hpp"
#include <stdexcept>
#include <memory>

namespace KapMirror {
    template <typename T>
    class ArraySegment {
      private:
        T* array;
        int offset;
        int size;

      public:
        ArraySegment() {
            array = nullptr;
            offset = 0;
            size = 0;
        }

        ArraySegment(T* _array, int _size) {
            array = Array::copyArray(_array, _size);
            offset = 0;
            size = _size;
        }

        ArraySegment(T* _array, int _offset, int _size) {
            array = Array::copyArray(_array, _size);
            offset = _offset;
            size = _size - _offset;
        }

        ~ArraySegment() { delete[] array; }

        /**
         * @brief Get the array
         * @return Array
         */
        T* toArray() const { return array + offset; }

        explicit operator T*() const { return array + offset; }

        /**
         * @brief Get the offset of the array
         * @return Offset
         */
        int getOffset() const { return offset; }

        /**
         * @brief Get the size of the array
         * @return Size
         */
        int getSize() const { return size; }

        T& operator[](int index) {
            if (index < 0 || index >= size) {
                throw std::out_of_range("index is out of range");
            }
            return array[offset + index];
        }

        bool operator==(ArraySegment<T> const& other) {
            for (int i = 0; i < std::min(size, other.size); i++) {
                if (array[offset + i] != other.array[other.offset + i]) {
                    return false;
                }
            }
            return true;
        }

        bool operator!=(ArraySegment<T> const& other) { return *this != other; }

        ArraySegment<T>& operator=(ArraySegment<T> const& other) {
            array = Array::copyArray(other.array, other.size);
            offset = other.offset;
            size = other.size;
            return *this;
        }

        static std::shared_ptr<ArraySegment<T>> createArraySegment(T* array, int size) {
            return std::make_shared<ArraySegment<T>>(array, size);
        }

        static std::shared_ptr<ArraySegment<T>> createArraySegment(T* array, int offset, int size) {
            return std::make_shared<ArraySegment<T>>(array, offset, size);
        }
    };
} // namespace KapMirror
