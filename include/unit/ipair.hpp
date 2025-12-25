/**
 * @file ipair.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines the base interface for paired units like Point and Size.
 * @version 0.1.0
 * @date 2025-12-25
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once

#include <algorithm>
#include <compare>
#include <cstdint>
#include <format>
#include <functional>
#include <limits>
#include <type_traits>
#include "../meta/concepts.hpp"

namespace frqs::widget {

template <meta::numeric Tb = int32_t>
class Point;

template <meta::numeric Tb = uint32_t>
class Size;

} // namespace frqs::widget

namespace frqs::meta {

/**
 * @brief Type trait to check if a type is a frqs::widget::Point.
 * @tparam T The type to check.
 */
template <typename> 
struct is_point : std::false_type {};

/**
 * @brief Specialization of is_point for frqs::widget::Point.
 */
template <meta::numeric Tb> 
struct is_point<::frqs::widget::Point<Tb>> : std::true_type {};

/**
 * @brief Helper variable template for is_point.
 * @tparam Tb The type to check.
 */
template <typename Tb>
constexpr bool is_point_v = is_point<std::decay_t<Tb>>::value;

/**
 * @brief Type trait to check if a type is a frqs::widget::Size.
 * @tparam T The type to check.
 */
template <typename> 
struct is_size : std::false_type {};

/**
 * @brief Specialization of is_size for frqs::widget::Size.
 */
template <meta::numeric Tb> 
struct is_size<::frqs::widget::Size<Tb>> : std::true_type {};

/**
 * @brief Helper variable template for is_size.
 * @tparam Tv The type to check.
 */
template <typename Tv>
constexpr bool is_size_v = is_size<std::decay_t<Tv>>::value;

/**
 * @brief Type trait to check if a type is a paired unit (Point or Size).
 * @tparam T The type to check.
 */
template <typename> 
struct is_paired_unit : std::false_type {};

/**
 * @brief Specialization of is_paired_unit for frqs::widget::Point.
 */
template <meta::numeric Tb>
struct is_paired_unit<::frqs::widget::Point<Tb>> : std::true_type {};

/**
 * @brief Specialization of is_paired_unit for frqs::widget::Size.
 */
template <meta::numeric Tb>
struct is_paired_unit<::frqs::widget::Size<Tb>> : std::true_type {};

/**
 * @brief Helper variable template for is_paired_unit.
 * @tparam Tv The type to check.
 */
template <typename Tv>
constexpr bool is_paired_unit_v = is_paired_unit<std::decay_t<Tv>>::value;

/**
 * @brief Concept that is satisfied if a type is a paired unit (Point or Size).
 * @tparam Tv The type to check.
 */
template <typename Tv>
concept paired_unit = is_paired_unit_v<Tv>;

} // namespace frqs::meta

namespace frqs::widget {

/**
 * @brief Interface for paired unit types like Point and Size (using CRTP).
 *        Provides common arithmetic operations and conversions.
 * @tparam Td The derived paired unit type (e.g., Point, Size).
 */
template <meta::paired_unit Td>
class IPair {
public:
	/** @brief The derived class type. */
	using child_t = Td;
	/** @brief The type of this class. */
	using self_t = IPair<Td>;

private:
	/**
	 * @brief Gets a const reference to the derived child object.
	 * @return A const reference to the child object.
	 */
	constexpr const Td& child() const noexcept { return static_cast<const Td&>(*this); }
	/**
	 * @brief Gets a reference to the derived child object.
	 * @return A reference to the child object.
	 */
	constexpr Td& child() noexcept { return static_cast<Td&>(*this); }

	/**
	 * @brief Clamps a numeric value to be non-negative. Used for Size components.
	 * @tparam Tv The numeric type.
	 * @param val The value to clamp.
	 * @return The clamped value.
	 */
	template <meta::numeric Tv>
	constexpr auto inClamp(Tv val) const noexcept {
        return std::clamp(val, Tv{0}, std::numeric_limits<Tv>::max());
	}

	/**
	 * @brief Adapts a binary operation for different operand types (paired units or numerics).
	 * @tparam Tv The type of the other operand.
	 * @tparam Fn The function object type for the operation (e.g., std::plus).
	 * @param val The other operand.
	 * @param fn The binary operation function.
	 * @return The result of the operation, as a new Point or Size.
	 */
	template <typename Tv, typename Fn>
	requires (meta::is_paired_unit_v<Tv> || meta::numeric<Tv>)
	constexpr auto OpAdapt(const Tv& val, Fn&& fn) const noexcept {
		if constexpr (std::is_arithmetic_v<Tv>) {
			if constexpr (meta::is_point<Td>::value)
				return Point(fn(child().x, val), fn(child().y, val));
			else 
				return Size(inClamp(fn(child().w, val)), inClamp(fn(child().h, val)));
		} else if constexpr (meta::is_point<Td>::value) {
			if constexpr (meta::is_point<Tv>::value) 
                return Point(fn(child().x, val.x), fn(child().y, val.y));
			else
            	return Point(fn(child().x, val.w), fn(child().y, val.h));
		} else { 
			if constexpr (meta::is_point<Tv>::value) 
                return Point(fn(child().w, val.x), fn(child().h, val.y));
			else
            	return Size(inClamp(fn(child().w, val.w)), inClamp(fn(child().h, val.h)));
		}
	}

public:
	/**
	 * @brief Defaulted three-way comparison operator.
	 * @param o The other IPair to compare to.
	 * @return The result of the comparison.
	 */
	constexpr std::strong_ordering operator<=>(const self_t& o) const noexcept = default;
	
    /**
     * @brief Conversion operator between different paired unit types (e.g., Point to Size).
     * @tparam Tdo The paired unit type to convert to.
     * @return The converted paired unit object.
     */
	template <meta::paired_unit Tdo>
	constexpr operator Tdo() const noexcept {
		if constexpr (meta::is_point<Td>::value)
			return Tdo(child().x, child().y);
		else 
			return Tdo(child().w, child().h);
	}

    /**
     * @brief Addition operator.
     * @tparam Tv A paired unit or numeric type.
     * @param o The right-hand side operand.
     * @return The result of the addition.
     */
	template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || meta::numeric<Tv>)
	constexpr auto operator+(const Tv& o) const noexcept {
		return OpAdapt(o, std::plus{});
	}

    /**
     * @brief Subtraction operator.
     * @tparam Tv A paired unit or numeric type.
     * @param o The right-hand side operand.
     * @return The result of the subtraction.
     */
	template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || meta::numeric<Tv>)
	constexpr auto operator-(const Tv& o) const noexcept {
		return OpAdapt(o, std::minus{});
	}

    /**
     * @brief Multiplication operator.
     * @tparam Tv A paired unit or numeric type.
     * @param o The right-hand side operand.
     * @return The result of the multiplication.
     */
	template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || meta::numeric<Tv>)
	constexpr auto operator*(const Tv& o) const noexcept {
		return OpAdapt(o, std::multiplies{});
	}

    /**
     * @brief Division operator.
     * @tparam Tv A paired unit or numeric type.
     * @param o The right-hand side operand.
     * @return The result of the division.
     */
	template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || meta::numeric<Tv>)
	constexpr auto operator/(const Tv& o) const noexcept {
		return OpAdapt(o, std::divides{});
	}

    /**
     * @brief Addition assignment operator.
     * @tparam Tv A paired unit or numeric type.
     * @param o The right-hand side operand.
     * @return A reference to the modified object.
     */
	template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || meta::numeric<Tv>)
	constexpr Td& operator+=(const Tv& o) noexcept {
		child() = child() + o;
		return child();
	}

    /**
     * @brief Subtraction assignment operator.
     * @tparam Tdo A paired unit or numeric type.
     * @param o The right-hand side operand.
     * @return A reference to the modified object.
     */
	template <typename Tdo>
	requires (meta::is_paired_unit_v<Tdo> || meta::numeric<Tdo>)
	constexpr Td& operator-=(const Tdo& o) noexcept {
		child() = child() - o;
		return child();
	}

    /**
     * @brief Multiplication assignment operator.
     * @tparam Tv A paired unit or numeric type.
     * @param o The right-hand side operand.
     * @return A reference to the modified object.
     */
	template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || meta::numeric<Tv>)
	constexpr Td& operator*=(const Tv& o) noexcept {
		child() = child() * o;
		return child();
	}

    /**
     * @brief Division assignment operator.
     * @tparam Tv A paired unit or numeric type.
     * @param o The right-hand side operand.
     * @return A reference to the modified object.
     */
	template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || meta::numeric<Tv>)
	constexpr Td& operator/=(const Tv& o) noexcept {
		child() = child() / o;
		return child();
	}

	/**
	 * @brief Generates a debug string representation of the object.
	 * @return A string in the format "Point(x: ..., y: ...)" or "Size(w: ..., h: ...)".
	 */
	inline std::string debug() const noexcept {
		if constexpr (meta::is_point<Td>::value)
			return std::format("Point(x: {}, y: {})", child().x, child().y);
		else 
			return std::format("Size(w: {}, h: {})", child().w, child().h);
	}
};

/**
 * @brief Swapped addition operator for (numeric + paired_unit).
 */
template <meta::numeric Lhs, meta::paired_unit Rhs>
constexpr auto operator+(Lhs lhs, const Rhs& rhs) noexcept { return rhs + lhs; }

/**
 * @brief Swapped multiplication operator for (numeric * paired_unit).
 */
template <meta::numeric Lhs, meta::paired_unit Rhs>
constexpr auto operator*(Lhs lhs, const Rhs& rhs) noexcept { return rhs * lhs; }

/**
 * @brief Swapped subtraction operator for (numeric - paired_unit).
 */
template <meta::numeric Lhs, meta::paired_unit Rhs>
constexpr auto operator-(Lhs lhs, const Rhs& rhs) noexcept { return Rhs(lhs) - rhs; }

/**
 * @brief Swapped division operator for (numeric / paired_unit).
 */
template <meta::numeric Lhs, meta::paired_unit Rhs>
constexpr auto operator/(Lhs lhs, const Rhs& rhs) noexcept { return Rhs(lhs) / rhs; }

} // namespace frqs::widget