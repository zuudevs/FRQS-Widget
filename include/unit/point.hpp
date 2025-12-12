#pragma once

#include "interface_paired_unit_fwd.hpp"
#include <compare>
#include <type_traits>

namespace frqs::widget {

template <typename Tv>
requires (std::is_integral_v<Tv> || std::is_floating_point_v<Tv>)
class Point {
public :
	using value_t = Tv ;

	Tv x {0} ;
	Tv y {0} ;

	constexpr Point() noexcept = default ;
	constexpr Point(const Point&) noexcept = default ;
	constexpr Point(Point&&) noexcept = default ;
	constexpr Point& operator=(const Point&) noexcept = default ;
	constexpr Point& operator=(Point&&) noexcept = default ;
	constexpr std::strong_ordering operator<=>(const Point&) const noexcept = default ;
	constexpr ~Point() noexcept = default ;

	template <meta::arithmetic Val>
	constexpr explicit Point(Val val) noexcept
	 : x(static_cast<Tv>(val)), y(static_cast<Tv>(val)) {}

	template <meta::arithmetic Vx, meta::arithmetic Vy>
	constexpr Point(Vx vx, Vy vy) noexcept
	 : x(static_cast<Tv>(vx)), y(static_cast<Tv>(vy)) {}

	template <meta::arithmetic Val>
	constexpr Point& operator=(Val val) noexcept {
		x = y = static_cast<Tv>(val) ;
		return *this ;
	}
} ;

template <typename Tv>
Point(Tv) -> Point<Tv> ;

template <typename Tx, typename Tv>
Point(Tx, Tv) -> Point<std::common_type_t<Tx, Tv>> ;

} // namespace frqs::widget