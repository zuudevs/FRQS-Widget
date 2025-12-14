#pragma once

#include <type_traits>

namespace frqs::meta {

template <typename Tv>
concept arithmetic = std::is_integral_v<Tv> || std::is_floating_point_v<Tv> ;

} // namespace frqs::meta