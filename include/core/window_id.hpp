/**
 * @file window_id.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2025-12-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <cstdint>
#include <functional>
#include <compare>

namespace frqs::core {

// ============================================================================
// WINDOW ID TYPE (Strong Type for Safety)
// ============================================================================

/**
 * @struct WindowId
 * @brief A strongly-typed unique identifier for a window.
 *
 * Using a struct instead of a raw integer (like `uint64_t`) prevents
 * accidental type mismatches. For example, you cannot accidentally pass a
 * widget ID to a function expecting a window ID.
 *
 * It is fully comparable and hashable, allowing it to be used as a key in
 * standard containers like `std::map` and `std::unordered_map`.
 */
struct WindowId {
    /** @brief The underlying integer value of the ID. */
    uint64_t value;
    
    /**
     * @brief Default comparison operators.
     * @details Enables direct comparison (==, !=, <, >, etc.) between two WindowId objects.
     */
    constexpr bool operator==(const WindowId&) const noexcept = default;
    constexpr auto operator<=>(const WindowId&) const noexcept = default;
};

} // namespace frqs::core

// ============================================================================
// HASH SUPPORT (for use in unordered containers)
// ============================================================================

/**
 * @brief Specialization of `std::hash` for `frqs::core::WindowId`.
 *
 * This allows `WindowId` objects to be used as keys in standard unordered
 * containers like `std::unordered_map` and `std::unordered_set`.
 */
template <>
struct std::hash<frqs::core::WindowId> {
    /**
     * @brief Computes the hash of a WindowId.
     * @param id The ID to hash.
     * @return The computed hash value.
     */
    std::size_t operator()(const frqs::core::WindowId& id) const noexcept {
        // The hash is simply the hash of the underlying integer value.
        return std::hash<uint64_t>{}(id.value);
    }
};