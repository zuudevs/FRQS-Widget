/**
 * @file color.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines color structures and concepts for the widget library.
 * @version 0.1
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "meta/concepts.hpp"
#include <compare>
#include <concepts>
#include <cstdint>
#include <algorithm>
#include <format>

namespace frqs::widget {

class Color;
class ColorF;

} // namespace frqs::widget

namespace frqs::meta {

/**
 * @brief Type trait to check if a type is a color type.
 * @tparam T The type to check.
 */
template <typename>
struct is_color : std::false_type {};

/**
 * @brief Specialization of is_color for frqs::widget::Color.
 */
template <>
struct is_color<::frqs::widget::Color> : std::true_type {};

/**
 * @brief Specialization of is_color for frqs::widget::ColorF.
 */
template <>
struct is_color<::frqs::widget::ColorF> : std::true_type {};

/**
 * @brief Helper variable template for is_color.
 * @tparam Tv The type to check.
 */
template <typename Tv>
constexpr bool is_color_v = is_color<std::decay_t<Tv>>::value;

/**
 * @brief Concept that is satisfied if a type is a color type.
 * @tparam Tv The type to check.
 */
template <typename Tv>
concept ColorType = is_color_v<Tv>;

} // namespace frqs::meta

namespace frqs::widget {

/**
 * @brief Interface for color types.
 * @tparam Td The derived color type (CRTP).
 */
template <meta::ColorType Td> 
class IColor {
public:
	/** @brief The derived class type. */
	using child_t = Td;
	/** @brief The type of this class. */
	using self_t = IColor<Td>;

protected:
	/**
	 * @brief Gets a reference to the derived child object.
	 * @return A reference to the child object.
	 */
	constexpr Td& child() noexcept {
		return static_cast<Td&>(*this);
	}

	/**
	 * @brief Gets a const reference to the derived child object.
	 * @return A const reference to the child object.
	 */
	constexpr const Td& child() const noexcept {
		return static_cast<const Td&>(*this);
	}

	/**
	 * @brief Safely casts a numeric value to the appropriate channel type.
     *        For Color (uint8_t), clamps the value between 0 and 255.
     *        For ColorF (float), clamps the value between 0.0 and 1.0.
	 * @tparam Tv The numeric type of the value.
	 * @param val The value to cast.
	 * @return The safely casted value.
	 */
	template <meta::numeric Tv>
    constexpr auto safeCast(Tv val) const noexcept {
		if constexpr (std::is_same_v<Td, Color>) {
			return static_cast<uint8_t>(std::clamp<Tv>(val, Tv{0}, Tv{255}));
		} else {
			return static_cast<float>(std::clamp<Tv>(val, Tv{0}, Tv{1}));
		}
    }

public:
	/**
	 * @brief Defaulted three-way comparison operator.
	 * @param o The other IColor to compare to.
	 * @return The result of the comparison.
	 */
	constexpr std::strong_ordering operator<=>(const self_t& o) const noexcept = default;

	/**
	 * @brief Conversion operator between different color types (Color and ColorF).
	 * @tparam To The color type to convert to.
	 * @return The converted color object.
	 */
	template <meta::ColorType To>
	constexpr operator To() const noexcept {
		if constexpr (std::is_same_v<To, Td>) {
			return To{child().r, child().g, child().b, child().a};
		} else if constexpr (std::is_same_v<Td, Color> && std::is_same_v<To, ColorF>) {
			return To{
				static_cast<float>(child().r) / 255.0f, 
				static_cast<float>(child().g) / 255.0f, 
				static_cast<float>(child().b) / 255.0f, 
				static_cast<float>(child().a) / 255.0f
			};
		} else {
			return To{
				static_cast<uint8_t>(child().r * 255), 
				static_cast<uint8_t>(child().g * 255), 
				static_cast<uint8_t>(child().b * 255), 
				static_cast<uint8_t>(child().a * 255)
			};
		}
	}

	/**
	 * @brief Generates a debug string representation of the object.
	 * @return A string in the format "Color(r: ..., g: ..., b: ..., a: ...)" or "ColorF(r: ..., g: ..., b: ..., a: ...)".
	 */
	inline std::string debug() const noexcept {
		if constexpr (std::is_same_v<Td, Color>)
			return std::format("Color(r: {}, g: {}, b: {}, a: {})", child().r, child().g, child().b, child().a);
		else 
			return std::format("ColorF(r: {}, g: {}, b: {}, a: {})", child().r, child().g, child().b, child().a);
	}
};

/**
 * @brief Represents a color with 8-bit integer channels (RGBA).
 */
class Color : public IColor<Color> {
public:
    /** @brief The underlying value type for each channel (uint8_t). */
    using value_t = uint8_t;

public:
    /** @brief The red component. */
    uint8_t r {0};
    /** @brief The green component. */
    uint8_t g {0};
    /** @brief The blue component. */
    uint8_t b {0};
    /** @brief The alpha (opacity) component. */
    uint8_t a {255};

    /** @brief Default constructor. Initializes to black. */
    constexpr Color() noexcept = default;
    /** @brief Copy constructor. */
    constexpr Color(const Color&) noexcept = default;
    /** @brief Move constructor. */
    constexpr Color(Color&&) noexcept = default;
    /** @brief Copy assignment operator. */
    constexpr Color& operator=(const Color&) noexcept = default;
    /** @brief Move assignment operator. */
    constexpr Color& operator=(Color&&) noexcept = default;
	/** @brief Three-way comparison operator. */
	constexpr std::strong_ordering operator<=>(const Color&) const noexcept = default ;
    /** @brief Destructor. */
    constexpr ~Color() noexcept = default;

    /**
     * @brief Constructs a grayscale color from a single integral value.
     * @tparam Tv An integral type.
     * @param val The value for R, G, and B components, clamped to [0, 255].
     */
    template <std::integral Tv>
    constexpr explicit Color(Tv val) noexcept
     : r(safeCast(val)), g(safeCast(val)), b(safeCast(val)) {}

    /**
     * @brief Constructs a color from R, G, B, and optional A integral values.
     * @tparam Tr An integral type for red.
     * @tparam Tg An integral type for green.
     * @tparam Tb An integral type for blue.
     * @tparam Ta An integral type for alpha.
     * @param tr The red component, clamped to [0, 255].
     * @param tg The green component, clamped to [0, 255].
     * @param tb The blue component, clamped to [0, 255].
     * @param ta The alpha component (default 255), clamped to [0, 255].
     */
    template <std::integral Tr, std::integral Tg, std::integral Tb, std::integral Ta = int>
    constexpr Color(Tr tr, Tg tg, Tb tb, Ta ta = 255) noexcept
     : r(safeCast(tr)), g(safeCast(tg)), b(safeCast(tb)), a(safeCast(ta)) {}

    /**
     * @brief Assigns a grayscale value to the color.
     * @tparam Tv An integral type.
     * @param val The value for R, G, and B components, clamped to [0, 255]. Alpha is unchanged.
     * @return A reference to this Color object.
     */
    template <std::integral Tv>
    constexpr Color& operator=(Tv val) noexcept {
        r = g = b = safeCast(val);
        return *this;
    }
};

/**
 * @brief Represents a color with floating-point channels (RGBA).
 *        Values are typically in the range [0.0, 1.0].
 */
class ColorF : public IColor<ColorF> {
public:
	/** @brief The underlying value type for each channel (float). */
	using value_t = float;

    /** @brief The red component. */
	float r {0.0f};
    /** @brief The green component. */
	float g {0.0f};
    /** @brief The blue component. */
	float b {0.0f};
    /** @brief The alpha (opacity) component. */
	float a {1.0f};

    /** @brief Default constructor. Initializes to black. */
	constexpr ColorF() noexcept = default;
    /** @brief Copy constructor. */
	constexpr ColorF(const ColorF&) noexcept = default;
    /** @brief Move constructor. */
	constexpr ColorF(ColorF&&) noexcept = default;
    /** @brief Copy assignment operator. */
	constexpr ColorF& operator=(const ColorF&) noexcept = default;
    /** @brief Move assignment operator. */
	constexpr ColorF& operator=(ColorF&&) noexcept = default;
	/** @brief Three-way comparison operator. */
	constexpr std::strong_ordering operator<=>(const ColorF&) const noexcept = default ;
    /** @brief Destructor. */
	constexpr ~ColorF() noexcept = default;

	/**
     * @brief Constructs a grayscale color from a single floating point value.
     * @tparam Tv A floating point type.
     * @param val The value for R, G, and B components, clamped to [0.0, 1.0].
     */
	template <std::floating_point Tv>
	constexpr explicit ColorF(Tv val) noexcept
	 : r(safeCast(val)), g(safeCast(val)), b(safeCast(val)) {}

	/**
     * @brief Constructs a color from R, G, B, and optional A floating point values.
     * @tparam Tr A floating point type for red.
     * @tparam Tg A floating point type for green.
     * @tparam Tb A floating point type for blue.
     * @tparam Ta A floating point type for alpha.
     * @param tr The red component, clamped to [0.0, 1.0].
     * @param tg The green component, clamped to [0.0, 1.0].
     * @param tb The blue component, clamped to [0.0, 1.0].
     * @param ta The alpha component (default 1.0), clamped to [0.0, 1.0].
     */
	template <std::floating_point Tr, std::floating_point Tg, std::floating_point Tb, std::floating_point Ta = float>
	constexpr ColorF(Tr tr, Tg tg, Tb tb, Ta ta = 1.0f) noexcept
	 : r(safeCast(tr)), g(safeCast(tg)), b(safeCast(tb)), a(safeCast(ta)) {}

	/**
     * @brief Assigns a grayscale value to the color.
     * @tparam Tv A floating point type.
     * @param val The value for R, G, and B components, clamped to [0.0, 1.0]. Alpha is unchanged.
     * @return A reference to this ColorF object.
     */
	template <std::floating_point Tv>
	constexpr ColorF& operator=(Tv val) noexcept {
		r = g = b = safeCast(val);
		return *this;
	}
};

/**
 * @brief A collection of predefined common colors.
 */
namespace colors {
    /** @brief Predefined white color (255, 255, 255). */
    static inline constexpr Color White {255};
    /** @brief Predefined black color (0, 0, 0). */
    static inline constexpr Color Black {0};
    /** @brief Predefined transparent color (0, 0, 0, 0). */
    static inline constexpr Color Transparent {0, 0, 0, 0};
    /** @brief Predefined red color (255, 0, 0). */
    static inline constexpr Color Red {255, 0, 0};
    /** @brief Predefined green color (0, 255, 0). */
    static inline constexpr Color Green {0, 255, 0};
    /** @brief Predefined blue color (0, 0, 255). */
    static inline constexpr Color Blue {0, 0, 255};

    /** @brief Predefined white color (1.0f, 1.0f, 1.0f). */
	static inline constexpr ColorF WhiteF {1.0f};
    /** @brief Predefined black color (0.0f, 0.0f, 0.0f). */
	static inline constexpr ColorF BlackF {0.0f};
    /** @brief Predefined transparent color (0.0f, 0.0f, 0.0f, 0.0f). */
    static inline constexpr ColorF TransparentF {0.0f, 0.0f, 0.0f, 0.0f};
    /** @brief Predefined red color (1.0f, 0.0f, 0.0f). */
    static inline constexpr ColorF RedF {1.0f, 0.0f, 0.0f};
    /** @brief Predefined green color (0.0f, 1.0f, 0.0f). */
    static inline constexpr ColorF GreenF {0.0f, 1.0f, 0.0f};
    /** @brief Predefined blue color (0.0f, 0.0f, 1.0f). */
    static inline constexpr ColorF BlueF {0.0f, 0.0f, 1.0f};
}

} // namespace frqs::widget