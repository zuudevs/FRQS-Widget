/**
 * @file size.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines a 2D Size class.
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
 * @brief Represents a 2D size with width and height dimensions.
 *
 * Inherits from IPair to provide common arithmetic operations.
 * Components are clamped to be non-negative in arithmetic operations within IPair.
 *
 * @tparam Tb The numeric type for the dimensions (e.g., uint32_t, float).
 */
template <meta::numeric Tb>
class Size : public IPair<Size<Tb>> {
public:
	/** @brief Friend declaration to allow IPair access to private members for CRTP. */
	friend class IPair<Size<Tb>>;
	/** @brief The value type of the dimensions. */
	using value_t = Tb;
	
	/** @brief The width component. */
	Tb w {0};
	/** @brief The height component. */
	Tb h {0};

	/** @brief Default constructor. Initializes dimensions to zero. */
	constexpr Size() noexcept = default;
	/** @brief Copy constructor. */
	constexpr Size(const Size&) noexcept = default;
	/** @brief Move constructor. */
	constexpr Size(Size&&) noexcept = default;
	/** @brief Copy assignment operator. */
	constexpr Size& operator=(const Size&) noexcept = default;
	/** @brief Move assignment operator. */
	constexpr Size& operator=(Size&&) noexcept = default;
	/** @brief Defaulted three-way comparison operator. */
	constexpr std::strong_ordering operator<=>(const Size&) const noexcept = default;
	/** @brief Destructor. */
	constexpr ~Size() noexcept = default;

	/**
	 * @brief Constructs a size with both width and height set to a single value.
	 * @tparam Tv A numeric type.
	 * @param val The value to assign to both w and h.
	 */
	template <meta::numeric Tv>
	constexpr explicit Size(Tv val) noexcept
	 : w(static_cast<Tb>(val)), h(static_cast<Tb>(val)) {}

	/**
	 * @brief Constructs a size from width and height values.
	 * @tparam Vx A numeric type for the width.
	 * @tparam Vy A numeric type for the height.
	 * @param vx The value for the width.
	 * @param vy The value for the height.
	 */
	template <meta::numeric Vx, meta::numeric Vy>
	constexpr Size(Vx vx, Vy vy) noexcept
	 : w(static_cast<Tb>(vx)), h(static_cast<Tb>(vy)) {}

	/**
	 * @brief Assigns a single numeric value to both width and height.
	 * @tparam Tv A numeric type.
	 * @param val The value to assign to w and h.
	 * @return A reference to this Size object.
	 */
	template <meta::numeric Tv>
	constexpr Size& operator=(Tv val) noexcept {
		w = h = static_cast<Tb>(val);
		return *this;
	}
};

/**
 * @brief Deduction guide for Size from a single numeric value.
 */
template <typename Tv>
Size(Tv) -> Size<Tv>;

/**
 * @brief Deduction guide for Size from two numeric values. Deduces the common type.
 */
template <typename Tx, typename Tv>
Size(Tx, Tv) -> Size<std::common_type_t<Tx, Tv>>;

} // namespace frqs::widget