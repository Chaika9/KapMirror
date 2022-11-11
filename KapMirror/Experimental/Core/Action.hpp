#pragma once

#include <map>
#include <functional>
#include <utility>
#include <mutex>

namespace KapMirror::Experimental {
    template <class Signature>
    struct Action {
      private:
        std::map<std::size_t, std::function<Signature>> handlers;
        std::mutex handlersMutex;

      public:
        /**
         * @brief Call all handlers
         * @param args The arguments to pass to the handlers
         */
        template <class... Args>
        void operator()(Args&&... args) {
            std::lock_guard<std::mutex> lock(handlersMutex);
            for (auto const& [hash, handler] : handlers) {
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
            std::size_t hash = getHash(hanlder);
            handlers[hash] = std::forward<Handler>(hanlder);
        }

        /**
         * @brief Remove a handler from the action
         * @param hanlder The handler to remove
         */
        template <typename Handler>
        void operator-=(Handler&& hanlder) {
            std::lock_guard<std::mutex> lock(handlersMutex);
            std::size_t hash = getHash(hanlder);
            handlers.erase(hash);
        }

        /**
         * @brief Clear all handlers
         */
        void clear() {
            std::lock_guard<std::mutex> lock(handlersMutex);
            handlers.clear();
        }

      private:
        std::size_t getHash(const std::function<Signature>& func) { return (std::size_t)func.target_type().hash_code(); }
    };
} // namespace KapMirror::Experimental
