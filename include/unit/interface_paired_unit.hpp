#pragma once

#include <algorithm>
#include <cstdint>
#include <format>
#include <limits>
#include <type_traits>

namespace frqs::widget {

template <typename Tv = int32_t>
requires (std::is_integral_v<Tv> || std::is_floating_point_v<Tv>)
class Point ;

template <typename Tv = uint32_t>
requires (std::is_integral_v<Tv> || std::is_floating_point_v<Tv>)
class Size ;

} // namespace frqs::widget

namespace frqs::meta {

template <typename Tv>
concept arithmetic = std::is_integral_v<Tv> || std::is_floating_point_v<Tv> ;

template <typename> 
struct is_point
 : std::false_type {} ;

template <typename Tv> 
requires (std::is_integral_v<Tv> || std::is_floating_point_v<Tv>)
struct is_point<widget::Point<Tv>>
 : std::false_type {} ;

template <typename> 
struct is_size
 : std::false_type {} ;

template <typename Tv> 
requires (std::is_integral_v<Tv> || std::is_floating_point_v<Tv>)
struct is_size<widget::Size<Tv>>
 : std::false_type {} ;

template <typename> 
struct is_paired_unit
 : std::false_type {} ;

template <typename Tv>
struct is_paired_unit<widget::Point<Tv>>
 : std::true_type {} ;

template <typename Tv>
struct is_paired_unit<widget::Size<Tv>>
 : std::true_type {} ;

template <typename Tv>
constexpr bool is_paired_unit_v = is_paired_unit<Tv>::value ;

template <typename Tv>
concept paired_unit = is_paired_unit_v<Tv> ;

} // namespace frqs::meta

namespace frqs::widget {

template <meta::paired_unit Td>
class IPair {
public :
	using child_t = Td ;
	using self_t = IPair<Td> ;

private :
	constexpr const Td& child() const noexcept { return static_cast<const Td&>(*this) ; }
	constexpr Td& child() noexcept { return static_cast<Td&>(*this) ; }

	template <typename Val, typename Min, typename Max>
	constexpr auto internalClamp(Val val, Min min, Max max) const noexcept {
        return std::clamp(val, static_cast<Val>(min), static_cast<Val>(max));
	}

	template <meta::paired_unit Tdo, typename Fn>
	constexpr auto OpAdapt(const Tdo& o, Fn&& fn) const noexcept {
		if constexpr (meta::is_point<Td>::value || meta::is_point<Tdo>::value) {
			return Point(fn(child().x, o.x), fn(child().y, o.y)) ;
		} else if constexpr (meta::is_size<Td>::value || meta::is_size<Tdo>::value) {
			using R = std::invoke_result_t<decltype(fn(child().w, o.w))> ;
			return Size(
				internalClamp(fn(child().w, o.w), 0, std::numeric_limits<R>::max()), 
				internalClamp(fn(child().h, o.h), 0, std::numeric_limits<R>::max())
			) ;
		}
		else if constexpr (meta::is_point<Td>::value) 
			return Point(fn(child().x, o.w), fn(child().y, o.h)) ;
		using R = std::invoke_result_t<decltype(fn(child().w, o.x))> ;
		return Size(
			internalClamp(fn(child().w, o.x), 0, std::numeric_limits<R>::max()), 
			internalClamp(fn(child().h, o.y), 0, std::numeric_limits<R>::max())
		) ;
	}

	template <meta::arithmetic Val, typename Fn>
	constexpr auto OpAdapt(Val val, Fn&& fn) const noexcept {
		if constexpr (meta::is_point<Td>::value) 
			return Point(fn(child().x, val), fn(child().y, val)) ;
		using R = std::invoke_result_t<decltype(fn(child().w, val))> ;
		return Size(
			internalClamp(fn(child().w, val), 0, std::numeric_limits<R>::max()), 
			internalClamp(fn(child().h, val), 0, std::numeric_limits<R>::max())
		) ;
	}

public :
	template <meta::paired_unit Tdo>
	constexpr auto operator+(const Tdo& o) const noexcept {
		return OpAdapt(o, [](auto a, auto b) { return a + b ; }) ;
	}

	template <meta::paired_unit Tdo>
	constexpr auto operator-(const Tdo& o) const noexcept {
		return OpAdapt(o, [](auto a, auto b) { return a - b ; }) ;
	}

	template <meta::paired_unit Tdo>
	constexpr auto operator*(const Tdo& o) const noexcept {
		return OpAdapt(o, [](auto a, auto b) { return a * b ; }) ;
	}

	template <meta::paired_unit Tdo>
	constexpr auto operator/(const Tdo& o) const noexcept {
		return OpAdapt(o, [](auto a, auto b) { return a / b ; }) ;
	}

	template <meta::arithmetic Val>
	constexpr auto operator+(Val val) const noexcept {
		return OpAdapt(val, [](auto a, auto b) { return a + b ; }) ;
	}

	template <meta::arithmetic Val>
	constexpr auto operator-(Val val) const noexcept {
		return OpAdapt(val, [](auto a, auto b) { return a - b ; }) ;
	}

	template <meta::arithmetic Val>
	constexpr auto operator*(Val val) const noexcept {
		return OpAdapt(val, [](auto a, auto b) { return a * b ; }) ;
	}

	template <meta::arithmetic Val>
	constexpr auto operator/(Val val) const noexcept {
		return OpAdapt(val, [](auto a, auto b) { return a / b ; }) ;
	}

	template <meta::paired_unit Tdo>
	constexpr Td& operator+=(const Tdo& o) noexcept {
		child() = child() + o ;
		return child() ;
	}

	template <meta::paired_unit Tdo>
	constexpr Td& operator-=(const Tdo& o) noexcept {
		child() = child() - o ;
		return child() ;
	}

	template <meta::paired_unit Tdo>
	constexpr Td& operator*=(const Tdo& o) noexcept {
		child() = child() * o ;
		return child() ;
	}

	template <meta::paired_unit Tdo>
	constexpr Td& operator/=(const Tdo& o) noexcept {
		child() = child() / o ;
		return child() ;
	}

	template <meta::arithmetic Val>
	constexpr auto operator+(Val val) noexcept {
		return OpAdapt(val, [](auto a, auto b) { return a + b ; }) ;
	}

	template <meta::arithmetic Val>
	constexpr auto operator-(Val val) noexcept {
		return OpAdapt(val, [](auto a, auto b) { return a - b ; }) ;
	}

	template <meta::arithmetic Val>
	constexpr auto operator*(Val val) noexcept {
		return OpAdapt(val, [](auto a, auto b) { return a * b ; }) ;
	}

	template <meta::arithmetic Val>
	constexpr auto operator/(Val val) noexcept {
		return OpAdapt(val, [](auto a, auto b) { return a / b ; }) ;
	}

	template <meta::arithmetic Val>
	constexpr Td& operator+=(Val val) noexcept {
		child() = child() + val ;
		return child() ;
	}

	template <meta::arithmetic Val>
	constexpr Td& operator-=(Val val) noexcept {
		child() = child() - val ;
		return child() ;
	}

	template <meta::arithmetic Val>
	constexpr Td& operator*=(Val val) noexcept {
		child() = child() * val ;
		return child() ;
	}

	template <meta::arithmetic Val>
	constexpr Td& operator/=(Val val) noexcept {
		child() = child() / val ;
		return child() ;
	}

	inline std::string debug() const noexcept {
		return (
			meta::is_point<Td>::value ? 
			std::format("Point(x: {}, y: {})", child().x, child().y) : 
			std::format("Size(w: {}, h: {})", child().w, child().h)
		) ;
	}
} ;

template <typename Lhs, typename Rhs>
requires (meta::is_paired_unit_v<Rhs>)
constexpr auto operator+(Lhs lhs, const Rhs& rhs) noexcept {
	return rhs + lhs ;
}

template <typename Lhs, typename Rhs>
requires (meta::is_paired_unit_v<Rhs>)
constexpr auto operator*(Lhs lhs, const Rhs& rhs) noexcept {
	return rhs * lhs ;
}

template <typename Lhs, typename Rhs>
requires (meta::is_paired_unit_v<Rhs>)
constexpr auto operator-(Lhs lhs, const Rhs& rhs) noexcept {
	return Rhs(lhs) - rhs ;
}

template <typename Lhs, typename Rhs>
requires (meta::is_paired_unit_v<Rhs>)
constexpr auto operator/(Lhs lhs, const Rhs& rhs) noexcept {
	return Rhs(lhs) / rhs ;
}

} // namespace frqs::widget