#pragma once

#include <list>
#include <functional>
#include <utility>
#include <mutex>

namespace KapMirror::Experimental {
    template <class Signature>
    struct Action {
      private:
        std::list<std::function<Signature>> handlers;
        std::mutex handlersMutex;

      public:
        /**
         * @brief Call all handlers
         * @param args The arguments to pass to the handlers
         */
        template <class... Args>
        void operator()(Args&&... args) {
            std::lock_guard<std::mutex> lock(handlersMutex);
            for (auto handler : handlers) {
                if (handler) {
                    handler(args...);
                }
            }
        }

        /**
         * @brief Add a handler to the action
         * @param hanlder The handler to add
         */
        template <typename Handler>
        void operator+=(Handler&& hanlder) {
            std::lock_guard<std::mutex> lock(handlersMutex);
            handlers.push_back(hanlder);
        }

        /**
         * @brief Clear all handlers
         */
        void clear() {
            std::lock_guard<std::mutex> lock(handlersMutex);
            handlers.clear();
        }
    };
} // namespace KapMirror::Experimental
