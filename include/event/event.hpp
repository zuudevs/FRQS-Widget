#pragma once

#include <variant>
#include <memory>
#include <vector>
#include <string>
#include "unit/rect.hpp"
#include "event_types.hpp"

namespace frqs::event {

// ============================================================================
// HYBRID VARIANT EVENT TYPE
// ============================================================================

using Event = std::variant<
    std::monostate,        // Empty/null event
    MouseMoveEvent,        // 32 bytes (hot)
    MouseButtonEvent,      // 24 bytes (hot)
    KeyEvent,              // 16 bytes (hot)
    ResizeEvent,           // 16 bytes (hot)
    PaintEvent,            // 16 bytes (hot)
    FileDropEvent,         // Variable (cold)
    MouseWheelEvent        // 24 bytes (hot)
>;

// Size validation (should be ~32-64 bytes on x64)
static_assert(sizeof(Event) <= 128, "Event variant too large for hot path");

// ============================================================================
// EVENT VISITOR HELPERS
// ============================================================================

template <typename... Handlers>
struct EventVisitor : Handlers... {
    using Handlers::operator()...;
};

template <typename... Handlers>
EventVisitor(Handlers...) -> EventVisitor<Handlers...>;

// ============================================================================
// EVENT TYPE CHECKS
// ============================================================================

template <typename T>
concept EventType = std::is_same_v<T, MouseMoveEvent> ||
                    std::is_same_v<T, MouseButtonEvent> ||
                    std::is_same_v<T, KeyEvent> ||
                    std::is_same_v<T, ResizeEvent> ||
                    std::is_same_v<T, PaintEvent> ||
                    std::is_same_v<T, FileDropEvent> ||
                    std::is_same_v<T, MouseWheelEvent>;

} // namespace frqs::event