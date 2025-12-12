#pragma once

#include "point.hpp"
#include "size.hpp"

namespace frqs::widget {

template <meta::paired_unit Td>
class IPair {
public :
	using child_t = Td ;
	using self_t = IPair<Td> ;

private :
	constexpr const Td& child() const noexcept { return static_cast<const Td*>(this) ; }
	constexpr Td& child() noexcept { return static_cast<Td*>(this) ; }

	template <meta::paired_unit Tdo, typename Fn>
	constexpr auto OpAdapt(const Tdo& o, Fn&& fn) const noexcept {
		if constexpr (meta::is_point<Td>::value || meta::is_point<Tdo>::value) 
			return Point(fn(child().x, o.x), fn(child().y, o.y)) ;
		else if constexpr (meta::is_size<Td>::value || meta::is_size<Tdo>::value) 
			return Size(fn(child().w, o.w), fn(child().h, o.h)) ;
		else if constexpr (meta::is_point<Td>::value) 
				return Point(fn(child().x, o.w), fn(child().y, o.h)) ;
		return Size(fn(child().w, o.x), fn(child().h, o.y)) ;
	}

	template <meta::arithmetic Val, typename Fn>
	constexpr auto OpAdapt(Val val, Fn&& fn) const noexcept {
		if constexpr (meta::is_point<Td>::value) 
			return Point(fn(child().x, val), fn(child().y, val)) ;
		return Size(fn(child().w, val), fn(child().h, val)) ;
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
	constexpr self_t& operator+=(const Tdo& o) noexcept {
		child() = child() + o ;
		return *this ;
	}

	template <meta::paired_unit Tdo>
	constexpr self_t& operator-=(const Tdo& o) noexcept {
		child() = child() - o ;
		return *this ;
	}

	template <meta::paired_unit Tdo>
	constexpr self_t& operator*=(const Tdo& o) noexcept {
		child() = child() * o ;
		return *this ;
	}

	template <meta::paired_unit Tdo>
	constexpr self_t& operator/=(const Tdo& o) noexcept {
		child() = child() / o ;
		return *this ;
	}

	template <meta::arithmetic Val>
	constexpr self_t& operator+=(Val val) noexcept {
		child() = child() + val ;
		return *this ;
	}

	template <meta::arithmetic Val>
	constexpr self_t& operator-=(Val val) noexcept {
		child() = child() - val ;
		return *this ;
	}

	template <meta::arithmetic Val>
	constexpr self_t& operator*=(Val val) noexcept {
		child() = child() * val ;
		return *this ;
	}

	template <meta::arithmetic Val>
	constexpr self_t& operator/=(Val val) noexcept {
		child() = child() / val ;
		return *this ;
	}
} ;

} // namespace frqs::widget