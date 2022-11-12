#pragma once

#include <chrono>

namespace KapMirror {
    class NetworkTime {
      public:
        /**
         * @brief Get the current time in milliseconds
         * @return Current time (in milliseconds)
         */
        static long long localTime() {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        }
    };
} // namespace KapMirror
