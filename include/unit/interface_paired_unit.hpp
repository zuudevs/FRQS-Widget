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
 : std::true_type {} ;

template <typename> 
struct is_size
 : std::false_type {} ;

template <typename Tv> 
requires (std::is_integral_v<Tv> || std::is_floating_point_v<Tv>)
struct is_size<widget::Size<Tv>>
 : std::true_type {} ;

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

	template <meta::arithmetic Val>
	constexpr auto inClamp(Val val) const noexcept {
        return std::clamp(val, Val{0}, std::numeric_limits<Val>::max()) ;
	}

	template <typename Val, typename Fn>
	requires (meta::is_paired_unit_v<Val> || meta::arithmetic<Val>)
	constexpr auto OpAdapt(const Val& val, Fn&& fn) const noexcept {
		if constexpr (std::is_arithmetic_v<Val>) {
			if constexpr (meta::is_point<Td>::value) {
				return Point(fn(child().x, val), fn(child().y, val)) ;
			}
			return Size(inClamp(fn(child().w, val)), inClamp(fn(child().h, val))) ;
		} else if constexpr (meta::is_point<Td>::value) {
			if constexpr (meta::is_point<Val>::value) {
                return Point(fn(child().x, val.x), fn(child().y, val.y)) ;
            }
            return Point(fn(child().x, val.w), fn(child().y, val.h)) ;
		} else { 
			if constexpr (meta::is_point<Val>::value) {
                return Point(fn(child().w, val.x), fn(child().h, val.y)) ;
            }
            return Size(inClamp(fn(child().w, val.w)), inClamp(fn(child().h, val.h))) ;
		}
	}

public :
	template <typename Val>
	requires (meta::is_paired_unit_v<Val> || meta::arithmetic<Val>)
	constexpr auto operator+(const Val& o) const noexcept {
		return OpAdapt(o, [](auto a, auto b) { return a + b ; }) ;
	}

	template <typename Val>
	requires (meta::is_paired_unit_v<Val> || meta::arithmetic<Val>)
	constexpr auto operator-(const Val& o) const noexcept {
		return OpAdapt(o, [](auto a, auto b) { return a - b ; }) ;
	}

	template <typename Val>
	requires (meta::is_paired_unit_v<Val> || meta::arithmetic<Val>)
	constexpr auto operator*(const Val& o) const noexcept {
		return OpAdapt(o, [](auto a, auto b) { return a * b ; }) ;
	}

	template <typename Val>
	requires (meta::is_paired_unit_v<Val> || meta::arithmetic<Val>)
	constexpr auto operator/(const Val& o) const noexcept {
		return OpAdapt(o, [](auto a, auto b) { return a / b ; }) ;
	}

	template <typename Val>
	requires (meta::is_paired_unit_v<Val> || meta::arithmetic<Val>)
	constexpr Td& operator+=(const Val& o) noexcept {
		child() = child() + o ;
		return child() ;
	}

	template <typename Tdo>
	requires (meta::is_paired_unit_v<Tdo> || meta::arithmetic<Tdo>)
	constexpr Td& operator-=(const Tdo& o) noexcept {
		child() = child() - o ;
		return child() ;
	}

	template <typename Val>
	requires (meta::is_paired_unit_v<Val> || meta::arithmetic<Val>)
	constexpr Td& operator*=(const Val& o) noexcept {
		child() = child() * o ;
		return child() ;
	}

	template <typename Val>
	requires (meta::is_paired_unit_v<Val> || meta::arithmetic<Val>)
	constexpr Td& operator/=(const Val& o) noexcept {
		child() = child() / o ;
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

template <meta::arithmetic Lhs, meta::paired_unit Rhs>
constexpr auto operator+(Lhs lhs, const Rhs& rhs) noexcept { return rhs + lhs ; }

template <meta::arithmetic Lhs, meta::paired_unit Rhs>
constexpr auto operator*(Lhs lhs, const Rhs& rhs) noexcept { return rhs * lhs ; }

template <meta::arithmetic Lhs, meta::paired_unit Rhs>
constexpr auto operator-(Lhs lhs, const Rhs& rhs) noexcept { return Rhs(lhs) - rhs ; }

template <meta::arithmetic Lhs, meta::paired_unit Rhs>
constexpr auto operator/(Lhs lhs, const Rhs& rhs) noexcept { return Rhs(lhs) / rhs ; }

} // namespace frqs::widget