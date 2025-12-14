#pragma once

#include <compare>
#include <concepts>
#include <cstdint>
#include <algorithm>

namespace frqs::widget {

class Color {
public :
    using value_t = uint8_t ;

private:
    template <typename T>
    constexpr uint8_t safeCast(T val) const noexcept {
        return static_cast<uint8_t>(std::clamp<T>(val, T{0}, T{255})) ;
    }

public :
    uint8_t r {0} ;
    uint8_t g {0} ;
    uint8_t b {0} ;
    uint8_t a {255} ;

    constexpr Color() noexcept = default ;
    constexpr Color(const Color&) noexcept = default ;
    constexpr Color(Color&&) noexcept = default ;
    constexpr Color& operator=(const Color&) noexcept = default ;
    constexpr Color& operator=(Color&&) noexcept = default ;
    constexpr std::strong_ordering operator<=>(const Color&) const noexcept = default ;
    constexpr ~Color() noexcept = default ;

    template <std::integral Tv>
    constexpr explicit Color(Tv val) noexcept
     : r(safeCast(val)), g(safeCast(val)), b(safeCast(val)) {}

    template <std::integral Tr, std::integral Tg, std::integral Tb, std::integral Ta = int>
    constexpr Color(Tr tr, Tg tg, Tb tb, Ta ta = 255) noexcept
     : r(safeCast(tr)), g(safeCast(tg)), b(safeCast(tb)), a(safeCast(ta)) {}

    template <std::integral Tv>
    constexpr Color& operator=(Tv val) noexcept {
        r = g = b = safeCast(val) ;
        return *this ;
    }
} ;

namespace colors {
    static inline constexpr Color White {255} ;
    static inline constexpr Color Black {0} ;
    static inline constexpr Color Transparent {0, 0, 0, 0} ;
    static inline constexpr Color Red {255, 0, 0} ;
    static inline constexpr Color Green {0, 255, 0} ;
    static inline constexpr Color Blue {0, 0, 255} ;
}

} // namespace frqs::widget