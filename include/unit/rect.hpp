#pragma once

#include "point.hpp"
#include "size.hpp"
#include <algorithm>

namespace frqs::widget {

template <meta::arithmetic Tp = int32_t, meta::arithmetic Ts = uint32_t>
class Rect ;

} // namespace frqs::widget

namespace frqs::meta {

template <typename>
struct is_rect : std::false_type {} ;

template <meta::arithmetic Tp, meta::arithmetic Ts>
struct is_rect<widget::Rect<Tp, Ts>> : std::true_type {} ;

template <typename Tv>
constexpr bool is_rect_v = is_rect<std::decay_t<Tv>>::value ;


} // namespace frqs::meta

namespace frqs::widget {

template <meta::arithmetic Tp, meta::arithmetic Ts>
class Rect : public Point<Tp>, public Size<Ts> {
public :
	using self_t = Rect<Tp, Ts> ;
    using point_value_t = Tp ;
    using size_value_t = Ts ;
    using base_point_t = Point<Tp> ; 
    using base_size_t = Size<Ts> ;

public :
    constexpr Rect() noexcept = default ;
	constexpr Rect(const Rect&) noexcept = default ;
	constexpr Rect(Rect&&) noexcept = default ;
	constexpr Rect& operator=(const Rect&) noexcept = default ;
	constexpr Rect& operator=(Rect&&) noexcept = default ;
	constexpr std::strong_ordering operator<=>(const Rect&) const noexcept = default ;
	constexpr ~Rect() noexcept = default ;
    

    template <meta::arithmetic Tv>
    constexpr explicit Rect(Tv val) noexcept
     : base_point_t(val), base_size_t(val) {}

    template <meta::arithmetic Tx, meta::arithmetic Ty, meta::arithmetic Tw, meta::arithmetic Th>
    constexpr Rect(Tx x, Ty y, Tw w, Th h) noexcept
     : base_point_t(x, y), base_size_t(w, h) {}

    template <typename Tpo, typename Tso>
    requires (meta::is_point_v<Tpo> && meta::is_size_v<Tso>)
    constexpr Rect(const Tpo& po, const Tso& so) noexcept
     : base_point_t(po), base_size_t(so) {}
    
    template <meta::arithmetic Tv>
    constexpr Rect& operator=(Tv val) noexcept {
        getPoint() = val ;
        getSize() = val ;
        return *this;
    }

    template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || std::is_arithmetic_v<Tv> || meta::is_rect_v<Tv>)
    constexpr auto operator+(Tv val) const noexcept {
		if constexpr (meta::is_rect_v<Tv>) 
			return Rect(getPoint() + val.getPoint(), getSize() + val.getSize()) ;
		else if constexpr (meta::is_point_v<Tv>) 
			return Rect(getPoint() + val, getSize()) ;
		else if constexpr (meta::is_size_v<Tv>) 
			return Rect(getPoint(), getSize() + val) ;
		else
        	return Rect(getPoint() + val, getSize() + val) ;
    }

    template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || std::is_arithmetic_v<Tv> || meta::is_rect_v<Tv>)
    constexpr auto operator-(Tv val) const noexcept {
		if constexpr (meta::is_rect_v<Tv>) 
			return Rect(getPoint() - val.getPoint(), getSize() - val.getSize()) ;
		else if constexpr (meta::is_point_v<Tv>) 
			return Rect(getPoint() - val, getSize()) ;
		else if constexpr (meta::is_size_v<Tv>) 
			return Rect(getPoint(), getSize() - val) ;
		else
        	return Rect(getPoint() - val, getSize() - val) ;
    }

	template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || std::is_arithmetic_v<Tv> || meta::is_rect_v<Tv>)
    constexpr auto operator*(Tv val) const noexcept {
		if constexpr (meta::is_rect_v<Tv>) 
			return Rect(getPoint() * val.getPoint(), getSize() * val.getSize()) ;
		else if constexpr (meta::is_point_v<Tv>) 
			return Rect(getPoint() * val, getSize()) ;
		else if constexpr (meta::is_size_v<Tv>) 
			return Rect(getPoint(), getSize() * val) ;
		else
        	return Rect(getPoint() * val, getSize() * val) ;
    }

    template <typename Tv>
	requires (meta::is_paired_unit_v<Tv> || std::is_arithmetic_v<Tv> || meta::is_rect_v<Tv>)
    constexpr auto operator/(Tv val) const noexcept {
		if constexpr (meta::is_rect_v<Tv>) 
			return Rect(getPoint() / val.getPoint(), getSize() / val.getSize()) ;
		else if constexpr (meta::is_point_v<Tv>) 
			return Rect(getPoint() / val, getSize()) ;
		else if constexpr (meta::is_size_v<Tv>) 
			return Rect(getPoint(), getSize() / val) ;
		else
        	return Rect(getPoint() / val, getSize() / val) ;
    }
    
    template <meta::arithmetic Tpo, meta::arithmetic Tso>
    constexpr operator Rect<Tpo, Tso>() const noexcept {
        return Rect<Tpo, Tso>(
            static_cast<Tpo>(this->x), static_cast<Tpo>(this->y),
            static_cast<Tso>(this->w), static_cast<Tso>(this->h)
        ) ;
    }

    constexpr auto getTop() const noexcept { return this->y ; }
    constexpr auto getLeft() const noexcept { return this->x ; }
    constexpr auto getRight() const noexcept { return this->x + this->w ; }
    constexpr auto getBottom() const noexcept { return this->y + this->h ; }
	constexpr auto getCenterX() const noexcept { return this->x + this->w / 2 ; }
	constexpr auto getCenterY() const noexcept { return this->y + this->h / 2 ; }
	constexpr auto getCenter() const noexcept { return Point(getCenterX(), getCenterY()) ; }
	constexpr const base_point_t& getPoint() const noexcept { return static_cast<const base_point_t&>(*this); }
    constexpr const base_size_t& getSize() const noexcept { return static_cast<const base_size_t&>(*this); }
    constexpr base_point_t& getPoint() noexcept { return static_cast<base_point_t&>(*this); }
    constexpr base_size_t& getSize() noexcept { return static_cast<base_size_t&>(*this); }
    
    constexpr auto intersect(const Rect& o) const noexcept {
        auto l = std::max(getLeft(), o.getLeft()) ;
        auto t = std::max(getTop(), o.getTop()) ;
        auto r = std::min(getRight(), o.getRight()) ;
        auto b = std::min(getBottom(), o.getBottom()) ;
        Ts w = (r > l) ? static_cast<Ts>(r - l) : Ts{0} ;
        Ts h = (b > t) ? static_cast<Ts>(b - t) : Ts{0} ;

        return Rect(l, t, w, h) ;
    }

	constexpr auto relativeTo(const Rect& o) const noexcept {
		return Rect(
			static_cast<point_value_t>(this->x - o.x),
			static_cast<point_value_t>(this->y - o.y),
			static_cast<size_value_t>(this->w),
			static_cast<size_value_t>(this->h)
		) ;
	}

	constexpr auto relativeFrom(const Rect& o) const noexcept {
		return Rect(
			static_cast<point_value_t>(this->x + o.x),
			static_cast<point_value_t>(this->y + o.y),
			static_cast<size_value_t>(this->w),
			static_cast<size_value_t>(this->h)
		) ;
	}

	[[nodiscard]] inline std::string debug() const noexcept {
		return std::format(
			"Rect(x: {}, y: {}, w: {}, h: {})",
			this->x, this->y, this->w, this->h
		) ;
	}
} ;

template <typename Tv>
Rect(Tv) -> Rect<Tv, Tv> ;

template <typename Tx, typename Ty, typename Tw, typename Th>
Rect(Tx, Ty, Tw, Th) -> Rect<std::common_type_t<Tx, Ty>, std::common_type_t<Tw, Th>> ;

template <typename Tp, typename Ts>
Rect(Tp, Ts) -> Rect<typename Tp::value_t, typename Ts::value_t> ;

} // namespace frqs::widget