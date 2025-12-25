/**
 * @file point.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines a 2D Point class.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "ipair.hpp"
#include <type_traits>

namespace frqs::widget {

/**
 * @brief Represents a 2D point with x and y coordinates.
 *
 * Inherits from IPair to provide common arithmetic operations.
 *
 * @tparam Tb The numeric type for the coordinates (e.g., int32_t, float).
 */
template <meta::numeric Tb>
class Point : public IPair<Point<Tb>> {
public:
	/** @brief The value type of the coordinates. */
	using value_t = Tb;

	/** @brief The x-coordinate of the point. */
	Tb x {0};
	/** @brief The y-coordinate of the point. */
	Tb y {0};

	/** @brief Default constructor. Initializes coordinates to zero. */
	constexpr Point() noexcept = default;
	/** @brief Copy constructor. */
	constexpr Point(const Point&) noexcept = default;
	/** @brief Move constructor. */
	constexpr Point(Point&&) noexcept = default;
	/** @brief Copy assignment operator. */
	constexpr Point& operator=(const Point&) noexcept = default;
	/** @brief Move assignment operator. */
	constexpr Point& operator=(Point&&) noexcept = default;
	/** @brief Defaulted three-way comparison operator. */
	constexpr std::strong_ordering operator<=>(const Point&) const noexcept = default;
	/** @brief Destructor. */
	constexpr ~Point() noexcept = default;

	/**
	 * @brief Constructs a point with both coordinates set to a single value.
	 * @tparam Tv A numeric type.
	 * @param val The value to assign to both x and y.
	 */
	template <meta::numeric Tv>
	constexpr explicit Point(Tv val) noexcept
	 : x(static_cast<Tb>(val)), y(static_cast<Tb>(val)) {}

	/**
	 * @brief Constructs a point from x and y coordinates.
	 * @tparam Tx A numeric type for the x-coordinate.
	 * @tparam Ty A numeric type for the y-coordinate.
	 * @param vx The value for the x-coordinate.
	 * @param vy The value for the y-coordinate.
	 */
	template <meta::numeric Tx, meta::numeric Ty>
	constexpr Point(Tx vx, Ty vy) noexcept
	 : x(static_cast<Tb>(vx)), y(static_cast<Tb>(vy)) {}

	/**
	 * @brief Assigns a single numeric value to both coordinates.
	 * @tparam Tv A numeric type.
	 * @param val The value to assign to x and y.
	 * @return A reference to this Point object.
	 */
	template <meta::numeric Tv>
	constexpr Point& operator=(Tv val) noexcept {
		x = y = static_cast<Tb>(val);
		return *this;
	}
};

/**
 * @brief Deduction guide for Point from a single numeric value.
 */
template <typename Tb>
Point(Tb) -> Point<Tb>;

/**
 * @brief Deduction guide for Point from two numeric values. Deduces the common type.
 */
template <typename Tx, typename Ty>
Point(Tx, Ty) -> Point<std::common_type_t<Tx, Ty>>;

} // namespace frqs::widget