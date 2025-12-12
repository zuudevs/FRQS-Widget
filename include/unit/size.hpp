#pragma once

#include "interface_paired_unit_fwd.hpp"
#include <compare>
#include <type_traits>

namespace frqs::widget {

template <typename Tv>
requires (std::is_integral_v<Tv> || std::is_floating_point_v<Tv>)
class Size {
public :
	friend class IPair<Size<Tv>> ;
	using value_t = Tv ;

private :
	template <meta::arithmetic Val, meta::arithmetic Min, meta::arithmetic Max>
	constexpr auto internalClamp(Val val, Max min, Min max) noexcept {
		return (
			val <= static_cast<Val>(min) ? static_cast<Val>(min) : (
				val >= static_cast<Val>(max) ? static_cast<Val>(max) : val
			)
		) ;
	}
	
public :
	Tv w {0} ;
	Tv h {0} ;

	constexpr Size() noexcept = default ;
	constexpr Size(const Size&) noexcept = default ;
	constexpr Size(Size&&) noexcept = default ;
	constexpr Size& operator=(const Size&) noexcept = default ;
	constexpr Size& operator=(Size&&) noexcept = default ;
	constexpr std::strong_ordering operator<=>(const Size&) const noexcept = default ;
	constexpr ~Size() noexcept = default ;

	template <meta::arithmetic Val>
	constexpr explicit Size(Val val) noexcept
	 : w(static_cast<Tv>(val)), h(static_cast<Tv>(val)) {}

	template <meta::arithmetic Vx, meta::arithmetic Vy>
	constexpr Size(Vx vx, Vy vy) noexcept
	 : w(static_cast<Tv>(vx)), h(static_cast<Tv>(vy)) {}

	template <meta::arithmetic Val>
	constexpr Size& operator=(Val val) noexcept {
		w = h = static_cast<Tv>(val) ;
		return *this ;
	}
} ;

template <typename Tv>
Size(Tv) -> Size<Tv> ;

template <typename Tx, typename Tv>
Size(Tx, Tv) -> Size<std::common_type_t<Tx, Tv>> ;

} // namespace frqs::widget