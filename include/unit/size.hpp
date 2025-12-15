#pragma once

#include "ipair.hpp"
#include <compare>
#include <type_traits>

namespace frqs::widget {

template <meta::numeric Tb>
class Size : public IPair<Size<Tb>> {
public :
	friend class IPair<Size<Tb>> ;
	using value_t = Tb ;
	
	Tb w {0} ;
	Tb h {0} ;

	constexpr Size() noexcept = default ;
	constexpr Size(const Size&) noexcept = default ;
	constexpr Size(Size&&) noexcept = default ;
	constexpr Size& operator=(const Size&) noexcept = default ;
	constexpr Size& operator=(Size&&) noexcept = default ;
	constexpr std::strong_ordering operator<=>(const Size&) const noexcept = default ;
	constexpr ~Size() noexcept = default ;

	template <meta::numeric Tv>
	constexpr explicit Size(Tv val) noexcept
	 : w(static_cast<Tb>(val)), h(static_cast<Tb>(val)) {}

	template <meta::numeric Vx, meta::numeric Vy>
	constexpr Size(Vx vx, Vy vy) noexcept
	 : w(static_cast<Tb>(vx)), h(static_cast<Tb>(vy)) {}

	template <meta::numeric Tv>
	constexpr Size& operator=(Tv val) noexcept {
		w = h = static_cast<Tb>(val) ;
		return *this ;
	}
} ;

template <typename Tv>
Size(Tv) -> Size<Tv> ;

template <typename Tx, typename Tv>
Size(Tx, Tv) -> Size<std::common_type_t<Tx, Tv>> ;

} // namespace frqs::widget