#pragma once

#include <variant>
#include <memory>
#include <vector>
#include <string>
#include "unit/rect.hpp"

namespace frqs::event {

// Forward declarations
struct MouseMoveEvent;
struct MouseButtonEvent;
struct KeyEvent;
struct ResizeEvent;
struct PaintEvent;
struct FileDropEvent;

// ============================================================================
// HOT PATH EVENTS (Small, Frequent) - Store as Values
// ============================================================================

struct MouseMoveEvent {
    widget::Point<int32_t> position;
    widget::Point<int32_t> delta;
    uint32_t modifiers; // Ctrl, Shift, Alt flags
    uint64_t timestamp;
};

struct MouseButtonEvent {
    enum class Button : uint8_t { Left, Right, Middle, X1, X2 };
    enum class Action : uint8_t { Press, Release, DoubleClick };
    
    Button button;
    Action action;
    widget::Point<int32_t> position;
    uint32_t modifiers;
    uint64_t timestamp;
};

struct MouseWheelEvent {
    int32_t delta;              // Scroll delta (typically ±120 per notch)
    widget::Point<int32_t> position;
    uint32_t modifiers;
    uint64_t timestamp;
};

struct KeyEvent {
    enum class Action : uint8_t { Press, Release, Repeat };
    
    uint32_t keyCode;
    Action action;
    uint32_t modifiers;
    uint64_t timestamp;
};

struct ResizeEvent {
    widget::Size<uint32_t> newSize;
    widget::Size<uint32_t> oldSize;
};

struct PaintEvent {
    widget::Rect<int32_t, uint32_t> dirtyRect;
};

// ============================================================================
// COLD PATH EVENTS (Large, Rare) - Store as unique_ptr
// ============================================================================

struct FileDropPayload {
    std::vector<std::wstring> filePaths;
    widget::Point<int32_t> dropPosition;
};

struct FileDropEvent {
    std::unique_ptr<FileDropPayload> payload;
    
    explicit FileDropEvent(std::vector<std::wstring> paths, widget::Point<int32_t> pos)
        : payload(std::make_unique<FileDropPayload>(
              FileDropPayload{std::move(paths), pos})) {}
};

// ============================================================================
// HYBRID VARIANT EVENT TYPE
// ============================================================================

using Event = std::variant<
    std::monostate,			// Empty/null event
    MouseMoveEvent,			// 32 bytes (hot)
    MouseButtonEvent,		// 24 bytes (hot)
    KeyEvent,				// 16 bytes (hot)
    ResizeEvent,			// 16 bytes (hot)
    PaintEvent,				// 16 bytes (hot)
    FileDropEvent,          // 8 bytes (ptr only, cold)
	MouseWheelEvent			// 24 bytes (hot)
>;

// Size validation (should be ~32-40 bytes on x64)
static_assert(sizeof(Event) <= 64, "Event variant too large for hot path");

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
                    std::is_same_v<T, FileDropEvent>;

} // namespace frqs::event