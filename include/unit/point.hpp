#pragma once

#include "ipair.hpp"
#include <compare>
#include <type_traits>

namespace frqs::widget {

template <meta::numeric Tb>
class Point : public IPair<Point<Tb>> {
public :
	using value_t = Tb ;

	Tb x {0} ;
	Tb y {0} ;

	constexpr Point() noexcept = default ;
	constexpr Point(const Point&) noexcept = default ;
	constexpr Point(Point&&) noexcept = default ;
	constexpr Point& operator=(const Point&) noexcept = default ;
	constexpr Point& operator=(Point&&) noexcept = default ;
	constexpr std::strong_ordering operator<=>(const Point&) const noexcept = default ;
	constexpr ~Point() noexcept = default ;

	template <meta::numeric Tv>
	constexpr explicit Point(Tv val) noexcept
	 : x(static_cast<Tb>(val)), y(static_cast<Tb>(val)) {}

	template <meta::numeric Tx, meta::numeric Ty>
	constexpr Point(Tx vx, Ty vy) noexcept
	 : x(static_cast<Tb>(vx)), y(static_cast<Tb>(vy)) {}

	template <meta::numeric Tv>
	constexpr Point& operator=(Tv val) noexcept {
		x = y = static_cast<Tb>(val) ;
		return *this ;
	}
} ;

template <typename Tb>
Point(Tb) -> Point<Tb> ;

template <typename Tx, typename Ty>
Point(Tx, Ty) -> Point<std::common_type_t<Tx, Ty>> ;

} // namespace frqs::widget