#pragma once

#include <map>
#include <functional>
#include <utility>

namespace KapMirror::Experimental {
    template <class Signature>
    struct Action {
      private:
        std::map<std::size_t, std::function<Signature>> handlers;

      public:
        template <class... Args>
        void operator()(Args&&... args) const {
            for (auto const& [hash, handler] : handlers) {
                if (handler) {
                    handler(args...);
                }
            }
        }

        template <typename Function>
        void operator+=(Function&& hanlder) {
            std::size_t hash = getHash(hanlder);
            handlers[hash] = std::forward<Function>(hanlder);
        }

        template <typename Function>
        void operator-=(Function&& hanlder) {
            std::size_t hash = getHash(hanlder);
            handlers.erase(hash);
        }

        void clear() { handlers.clear(); }

      private:
        std::size_t getHash(const std::function<Signature>& func) { return (std::size_t)func.target_type().hash_code(); }
    };
} // namespace KapMirror::Experimental
