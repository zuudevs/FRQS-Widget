#pragma once

#include <cstdint>
#include <functional>

namespace frqs::core {

// ============================================================================
// WINDOW ID TYPE (Strong Type for Safety)
// ============================================================================

struct WindowId {
    uint64_t value;
    
    constexpr bool operator==(const WindowId&) const noexcept = default;
    constexpr auto operator<=>(const WindowId&) const noexcept = default;
};

} // namespace frqs::core

// ============================================================================
// HASH SUPPORT (for use in unordered containers)
// ============================================================================

template <>
struct std::hash<frqs::core::WindowId> {
    std::size_t operator()(const frqs::core::WindowId& id) const noexcept {
        return std::hash<uint64_t>{}(id.value);
    }
};