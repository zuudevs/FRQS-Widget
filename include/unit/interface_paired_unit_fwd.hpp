#pragma once

#include <cstdint>
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
class IPair ;

} // namespace frqs::widget