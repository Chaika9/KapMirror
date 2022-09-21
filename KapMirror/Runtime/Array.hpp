#pragma once

#include <algorithm>

namespace Array {
    class Array {
        public:
        template <typename T>
        static void resizeArray(T*& array, int size, int newSize) {
            T* newArray = new T[newSize];

            std::copy(array, array + std::min(size, newSize), newArray);
            if (size < newSize) {
                std::fill(newArray + size, newArray + newSize, 0);
            }
            delete[] array;
            array = newArray;
        }

        template <typename T>
        static T *copyArray(T* array, int size) {
            T* newArray = new T[size];
            std::copy(array, array + size, newArray);
            return newArray;
        }
    };
}
