//
// Created by gbian on 25/07/2024.
//

#pragma once

#include <functional>

namespace lve {
    static inline constexpr auto golden_ratio = 0x9e3779b9;
    // from: https://stackoverflow.com/a/57595105
    template <typename T, typename... Rest> void hashCombine(std::size_t &seed, const T &v, const Rest &...rest) noexcept {
        seed ^= std::hash<T>{}(v) + golden_ratio + (seed << 6) + (seed >> 2);
        (hashCombine(seed, rest), ...);
    };

}  // namespace lve
