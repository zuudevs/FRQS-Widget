#pragma once

#include <algorithm>
#include <cstdint>
#include <format>
#include <functional>
#include <limits>
#include <type_traits>
#include "../meta/concepts.hpp"

namespace frqs::widget {

template <meta::numeric Tb = int32_t>
class Point ;

template <meta::numeric Tb = uint32_t>
class Size ;

} // namespace frqs::widget

namespace frqs::meta {

template <typename> 
struct is_point : std::false_type {} ;

template <meta::numeric Tb> 
struct is_point<::frqs::widget::Point<Tb>> : std::true_type {} ;

template <typename Tb>
constexpr bool is_point_v = is_point<std::decay_t<Tb>>::value ;

template <typename> 
struct is_size : std::false_type {} ;

template <meta::numeric Tb> 
struct is_size<::frqs::widget::Size<Tb>> : std::true_type {} ;

template <typename Tv>
constexpr bool is_size_v = is_size<std::decay_t<Tv>>::value ;

template <typename> 
struct is_paired_unit : std::false_type {} ;

template <meta::numeric Tb>
struct is_paired_unit<::frqs::widget::Point<Tb>> : std::true_type {} ;

template <meta::numeric Tb>
struct is_paired_unit<::frqs::widget::Size<Tb>> : std::true_type {} ;

template <typename Tv>
constexpr bool is_paired_unit_v = is_paired_unit<std::decay_t<Tv>>::value ;

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

	template <meta::numeric Tv>
	constexpr auto inClamp(Tv val) const noexcept {
        return std::clamp(val, Tv{0}, std::numeric_limits<Tv>::max()) ;
	}

	template <typename Tv, typename Fn>
	requires (meta::is_paired_unit_v<Tv> || meta::numeric<Tv>)
	constexpr auto OpAdapt(const Tv& val, Fn&& fn) const noexcept {
		if constexpr (std::is_arithmetic_v<Tv>) {
			if constexpr (meta::is_point<Td>::value)
				return Point(fn(child().x, val), fn(child().y, val)) ;
			else 
				return Size(inClamp(fn(child().w, val)), inClamp(fn(child().h, val))) ;
		} else if constexpr (meta::is_point<Td>::value) {
			if constexpr (meta::is_point<Tv>::value) 
                return Point(fn(child().x, val.x), fn(child().y, val.y)) ;
			else
            	return Point(fn(child().x, val.w), fn(child().y, val.h)) ;
		} else { 
			if constexpr (meta::is_point<Tv>::value) 
                return Point(fn(child().w, val.x), fn(child().h, val.y)) ;
			else
            	return Size(inClamp(fn(child().w, val.w)), inClamp(fn(child().h, val.h))) ;
		}
	}

public :
	template <meta::paired_unit Tdo>
	constexpr operator Tdo() const noexcept {
		if constexpr (meta::is_point<Td>::value)
			return Tdo(child().x, child().y) ;
		else 
			return Tdo(child().w, child().h) ;
	}

	template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || meta::numeric<Tv>)
	constexpr auto operator+(const Tv& o) const noexcept {
		return OpAdapt(o, std::plus{}) ;
	}

	template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || meta::numeric<Tv>)
	constexpr auto operator-(const Tv& o) const noexcept {
		return OpAdapt(o, std::minus{}) ;
	}

	template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || meta::numeric<Tv>)
	constexpr auto operator*(const Tv& o) const noexcept {
		return OpAdapt(o, std::multiplies{}) ;
	}

	template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || meta::numeric<Tv>)
	constexpr auto operator/(const Tv& o) const noexcept {
		return OpAdapt(o, std::divides{}) ;
	}

	template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || meta::numeric<Tv>)
	constexpr Td& operator+=(const Tv& o) noexcept {
		child() = child() + o ;
		return child() ;
	}

	template <typename Tdo>
	requires (meta::is_paired_unit_v<Tdo> || meta::numeric<Tdo>)
	constexpr Td& operator-=(const Tdo& o) noexcept {
		child() = child() - o ;
		return child() ;
	}

	template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || meta::numeric<Tv>)
	constexpr Td& operator*=(const Tv& o) noexcept {
		child() = child() * o ;
		return child() ;
	}

	template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || meta::numeric<Tv>)
	constexpr Td& operator/=(const Tv& o) noexcept {
		child() = child() / o ;
		return child() ;
	}

	inline std::string debug() const noexcept {
		if constexpr (meta::is_point<Td>::value)
			return std::format("Point(x: {}, y: {})", child().x, child().y) ;
		else 
			return std::format("Size(w: {}, h: {})", child().w, child().h) ;
	}
} ;

template <meta::numeric Lhs, meta::paired_unit Rhs>
constexpr auto operator+(Lhs lhs, const Rhs& rhs) noexcept { return rhs + lhs ; }

template <meta::numeric Lhs, meta::paired_unit Rhs>
constexpr auto operator*(Lhs lhs, const Rhs& rhs) noexcept { return rhs * lhs ; }

template <meta::numeric Lhs, meta::paired_unit Rhs>
constexpr auto operator-(Lhs lhs, const Rhs& rhs) noexcept { return Rhs(lhs) - rhs ; }

template <meta::numeric Lhs, meta::paired_unit Rhs>
constexpr auto operator/(Lhs lhs, const Rhs& rhs) noexcept { return Rhs(lhs) / rhs ; }

} // namespace frqs::widget