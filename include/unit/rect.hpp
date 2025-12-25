/**
 * @file rect.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines a Rectangle class with position and size.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "point.hpp"
#include "size.hpp"
#include <algorithm>

namespace frqs::widget {

/**
 * @brief Forward declaration of the Rect class.
 * @tparam Tp The numeric type for the point coordinates (e.g., int32_t, float).
 * @tparam Ts The numeric type for the size dimensions (e.g., uint32_t, float).
 */
template <meta::numeric Tp = int32_t, meta::numeric Ts = uint32_t>
class Rect;

} // namespace frqs::widget

namespace frqs::meta {

/**
 * @brief Type trait to check if a type is a frqs::widget::Rect.
 * @tparam T The type to check.
 */
template <typename>
struct is_rect : std::false_type {};

/**
 * @brief Specialization of is_rect for frqs::widget::Rect.
 */
template <meta::numeric Tp, meta::numeric Ts>
struct is_rect<::frqs::widget::Rect<Tp, Ts>> : std::true_type {};

/**
 * @brief Helper variable template for is_rect.
 * @tparam Tv The type to check.
 */
template <typename Tv>
constexpr bool is_rect_v = is_rect<std::decay_t<Tv>>::value;


} // namespace frqs::meta

namespace frqs::widget {

/**
 * @brief Represents a rectangle defined by a top-left point and a size.
 * 
 * Inherits from Point<Tp> and Size<Ts> to represent its position and dimensions.
 *
 * @tparam Tp The numeric type for the point coordinates (x, y).
 * @tparam Ts The numeric type for the size dimensions (w, h).
 */
template <meta::numeric Tp, meta::numeric Ts>
class Rect : public Point<Tp>, public Size<Ts> {
public:
	/** @brief The type of this class. */
	using self_t = Rect<Tp, Ts>;
    /** @brief The value type of the point components (x, y). */
    using point_value_t = Tp;
    /** @brief The value type of the size components (w, h). */
    using size_value_t = Ts;
    /** @brief The base Point type. */
    using base_point_t = Point<Tp>; 
    /** @brief The base Size type. */
    using base_size_t = Size<Ts>;

public:
    /** @brief Default constructor. Initializes point and size to zero. */
    constexpr Rect() noexcept = default;
	/** @brief Copy constructor. */
	constexpr Rect(const Rect&) noexcept = default;
	/** @brief Move constructor. */
	constexpr Rect(Rect&&) noexcept = default;
	/** @brief Copy assignment operator. */
	constexpr Rect& operator=(const Rect&) noexcept = default;
	/** @brief Move assignment operator. */
	constexpr Rect& operator=(Rect&&) noexcept = default;
	/** @brief Defaulted three-way comparison operator. */
	constexpr auto operator<=>(const Rect&) const noexcept = default;
	/** @brief Destructor. */
	constexpr ~Rect() noexcept = default;
    
    /**
     * @brief Constructs a rectangle with all components set to a single value.
     * @tparam Tv A numeric type.
     * @param val The value to assign to x, y, w, and h.
     */
    template <meta::numeric Tv>
    constexpr explicit Rect(Tv val) noexcept
     : base_point_t(val), base_size_t(val) {}

    /**
     * @brief Constructs a rectangle from x, y, width, and height values.
     * @tparam Tx Numeric type for x.
     * @tparam Ty Numeric type for y.
     * @tparam Tw Numeric type for width.
     * @tparam Th Numeric type for height.
     * @param x The x-coordinate of the top-left corner.
     * @param y The y-coordinate of the top-left corner.
     * @param w The width of the rectangle.
     * @param h The height of the rectangle.
     */
    template <meta::numeric Tx, meta::numeric Ty, meta::numeric Tw, meta::numeric Th>
    constexpr Rect(Tx x, Ty y, Tw w, Th h) noexcept
     : base_point_t(x, y), base_size_t(w, h) {}

    /**
     * @brief Constructs a rectangle from a Point and a Size object.
     * @tparam Tpo A Point type.
     * @tparam Tso A Size type.
     * @param po The top-left corner of the rectangle.
     * @param so The size of the rectangle.
     */
    template <typename Tpo, typename Tso>
    requires (meta::is_point_v<Tpo> && meta::is_size_v<Tso>)
    constexpr Rect(const Tpo& po, const Tso& so) noexcept
     : base_point_t(po), base_size_t(so) {}
    
    /**
     * @brief Assigns a single numeric value to all components of the rectangle.
     * @tparam Tv A numeric type.
     * @param val The value to assign.
     * @return A reference to this Rect object.
     */
    template <meta::numeric Tv>
    constexpr Rect& operator=(Tv val) noexcept {
        getPoint() = val;
        getSize() = val;
        return *this;
    }

    /**
     * @brief Adds another Rect, Point, Size, or numeric value.
     * @param val The value to add.
     * @return A new Rect representing the result.
     */
    template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || std::is_arithmetic_v<Tv> || meta::is_rect_v<Tv>)
    constexpr auto operator+(Tv val) const noexcept {
		if constexpr (meta::is_rect_v<Tv>) 
			return Rect(getPoint() + val.getPoint(), getSize() + val.getSize());
		else if constexpr (meta::is_point_v<Tv>) 
			return Rect(getPoint() + val, getSize());
		else if constexpr (meta::is_size_v<Tv>) 
			return Rect(getPoint(), getSize() + val);
		else
        	return Rect(getPoint() + val, getSize() + val);
    }

    /**
     * @brief Subtracts another Rect, Point, Size, or numeric value.
     * @param val The value to subtract.
     * @return A new Rect representing the result.
     */
    template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || std::is_arithmetic_v<Tv> || meta::is_rect_v<Tv>)
    constexpr auto operator-(Tv val) const noexcept {
		if constexpr (meta::is_rect_v<Tv>) 
			return Rect(getPoint() - val.getPoint(), getSize() - val.getSize());
		else if constexpr (meta::is_point_v<Tv>) 
			return Rect(getPoint() - val, getSize());
		else if constexpr (meta::is_size_v<Tv>) 
			return Rect(getPoint(), getSize() - val);
		else
        	return Rect(getPoint() - val, getSize() - val);
    }

	/**
     * @brief Multiplies by another Rect, Point, Size, or numeric value.
     * @param val The value to multiply by.
     * @return A new Rect representing the result.
     */
    template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || std::is_arithmetic_v<Tv> || meta::is_rect_v<Tv>)
    constexpr auto operator*(Tv val) const noexcept {
		if constexpr (meta::is_rect_v<Tv>) 
			return Rect(getPoint() * val.getPoint(), getSize() * val.getSize());
		else if constexpr (meta::is_point_v<Tv>) 
			return Rect(getPoint() * val, getSize());
		else if constexpr (meta::is_size_v<Tv>) 
			return Rect(getPoint(), getSize() * val);
		else
        	return Rect(getPoint() * val, getSize() * val);
    }

    /**
     * @brief Divides by another Rect, Point, Size, or numeric value.
     * @param val The value to divide by.
     * @return A new Rect representing the result.
     */
    template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || std::is_arithmetic_v<Tv> || meta::is_rect_v<Tv>)
    constexpr auto operator/(Tv val) const noexcept {
		if constexpr (meta::is_rect_v<Tv>) 
			return Rect(getPoint() / val.getPoint(), getSize() / val.getSize());
		else if constexpr (meta::is_point_v<Tv>) 
			return Rect(getPoint() / val, getSize());
		else if constexpr (meta::is_size_v<Tv>) 
			return Rect(getPoint(), getSize() / val);
		else
        	return Rect(getPoint() / val, getSize() / val);
    }
    
    /**
     * @brief Conversion operator to a Rect with different underlying numeric types.
     * @tparam Tpo The target point component type.
     * @tparam Tso The target size component type.
     * @return A new Rect with the converted types.
     */
    template <meta::numeric Tpo, meta::numeric Tso>
    constexpr operator Rect<Tpo, Tso>() const noexcept {
        return Rect<Tpo, Tso>(
            static_cast<Tpo>(this->x), static_cast<Tpo>(this->y),
            static_cast<Tso>(this->w), static_cast<Tso>(this->h)
        );
    }

    /** @brief Gets the top y-coordinate. */
    constexpr auto getTop() const noexcept { return this->y; }
    /** @brief Gets the left x-coordinate. */
    constexpr auto getLeft() const noexcept { return this->x; }
    /** @brief Gets the right x-coordinate (x + width). */
    constexpr auto getRight() const noexcept { return this->x + this->w; }
    /** @brief Gets the bottom y-coordinate (y + height). */
    constexpr auto getBottom() const noexcept { return this->y + this->h; }
	/** @brief Gets the center x-coordinate. */
	constexpr auto getCenterX() const noexcept { return this->x + this->w / 2; }
	/** @brief Gets the center y-coordinate. */
	constexpr auto getCenterY() const noexcept { return this->y + this->h / 2; }
	/** @brief Gets the center point of the rectangle. */
	constexpr auto getCenter() const noexcept { return Point(getCenterX(), getCenterY()); }
	/** @brief Gets a const reference to the base Point object. */
	constexpr const base_point_t& getPoint() const noexcept { return static_cast<const base_point_t&>(*this); }
    /** @brief Gets a const reference to the base Size object. */
    constexpr const base_size_t& getSize() const noexcept { return static_cast<const base_size_t&>(*this); }
    /** @brief Gets a reference to the base Point object. */
    constexpr base_point_t& getPoint() noexcept { return static_cast<base_point_t&>(*this); }
    /** @brief Gets a reference to the base Size object. */
    constexpr base_size_t& getSize() noexcept { return static_cast<base_size_t&>(*this); }

	/**
	 * @brief Checks if a point is contained within this rectangle.
	 * @tparam Tpo A Point type.
	 * @param pt The point to check.
	 * @return True if the point is inside the rectangle's bounds, false otherwise.
	 */
	template <typename Tpo>
	requires (meta::is_point_v<Tpo>)
	constexpr auto contain(const Tpo& pt) const noexcept {
		return pt.x >= getPoint().x
			 && pt.x < getPoint().x + getSize().w 
			 && pt.y >= getPoint().y 
			 && pt.y < getPoint().y + getSize().h;
	}
    
	/**
	 * @brief Calculates the intersection of this rectangle and another.
	 * @tparam To A Rect type.
	 * @param o The other rectangle.
	 * @return A new Rect representing the overlapping area. If no overlap, the resulting rect has zero width or height.
	 */
	template <typename To>
	requires (meta::is_rect_v<To>)
    constexpr auto intersect(const To& o) const noexcept {
        auto l = std::max(getLeft(), o.getLeft());
        auto t = std::max(getTop(), o.getTop());
        auto r = std::min(getRight(), o.getRight());
        auto b = std::min(getBottom(), o.getBottom());
        Ts w = (r > static_cast<uint32_t>(l)) ? static_cast<Ts>(r - l) : Ts{0};
        Ts h = (b > static_cast<uint32_t>(t)) ? static_cast<Ts>(b - t) : Ts{0};

        return Rect(l, t, w, h);
    }

	/**
	 * @brief Merges this rectangle with another to create a bounding rectangle.
	 * @param other The other rectangle to merge with.
	 * @return A new Rect that encloses both original rectangles.
	 */
	constexpr auto merge(const Rect& other) const noexcept {
        float x1 = std::min(getPoint().x, other.x);
        float y1 = std::min(getPoint().y, other.y);
        float x2 = std::max(getPoint().x + getSize().w, other.x + other.w);
        float y2 = std::max(getPoint().y + getSize().h, other.y + other.h);
        return Rect(x1, y1, x2 - x1, y2 - y1);
    }

	/**
	 * @brief Calculates this rectangle's position relative to another rectangle's origin.
	 * @param o The reference rectangle.
	 * @return A new Rect with its position shifted relative to 'o'.
	 */
	constexpr auto relativeTo(const Rect& o) const noexcept {
		return Rect(
			static_cast<point_value_t>(this->x - o.x),
			static_cast<point_value_t>(this->y - o.y),
			static_cast<size_value_t>(this->w),
			static_cast<size_value_t>(this->h)
		);
	}

	/**
	 * @brief Calculates this rectangle's absolute position from a relative position within another rectangle.
	 * @param o The parent/container rectangle.
	 * @return A new Rect with its position shifted by 'o's origin.
	 */
	constexpr auto relativeFrom(const Rect& o) const noexcept {
		return Rect(
			static_cast<point_value_t>(this->x + o.x),
			static_cast<point_value_t>(this->y + o.y),
			static_cast<size_value_t>(this->w),
			static_cast<size_value_t>(this->h)
		);
	}

	/**
	 * @brief Generates a debug string representation of the object.
     * @return A string in the format "Rect(x: ..., y: ..., w: ..., h: ...)".
	 */
	[[nodiscard]] inline std::string debug() const noexcept {
		return std::format(
			"Rect(x: {}, y: {}, w: {}, h: {})",
			this->x, this->y, this->w, this->h
		);
	}
};

/**
 * @brief Deduction guide for Rect from a single numeric value.
 */
template <typename Tv>
Rect(Tv) -> Rect<Tv, Tv>;

/**
 * @brief Deduction guide for Rect from four numeric values (x, y, w, h).
 */
template <typename Tx, typename Ty, typename Tw, typename Th>
Rect(Tx, Ty, Tw, Th) -> Rect<std::common_type_t<Tx, Ty>, std::common_type_t<Tw, Th>>;

/**
 * @brief Deduction guide for Rect from a Point and a Size object.
 */
template <typename Tp, typename Ts>
Rect(Tp, Ts) -> Rect<typename Tp::value_t, typename Ts::value_t>;

} // namespace frqs::widget