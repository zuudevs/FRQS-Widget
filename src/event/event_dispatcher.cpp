#include "../../include/event/event.hpp"
#include "../../include/widget/iwidget.hpp"
#include "../../include/core/window.hpp"
#include <chrono>

namespace frqs::event {

// ============================================================================
// EVENT DISPATCHER (Helper Functions)
// ============================================================================

// Dispatch event to widget tree (depth-first)
bool dispatchToWidgetTree(widget::IWidget* root, const Event& event) {
    if (!root || !root->isVisible()) {
        return false;
    }

    // Try children first (front-to-back for proper z-order)
    for (const auto& child : root->getChildren()) {
        if (dispatchToWidgetTree(child.get(), event)) {
            return true;  // Event handled by child
        }
    }

    // Try the widget itself
    return root->onEvent(event);
}

// Create mouse move event from native coordinates
MouseMoveEvent createMouseMoveEvent(
    int32_t x, int32_t y,
    int32_t prevX, int32_t prevY,
    uint32_t modifiers
) {
    return MouseMoveEvent{
        .position = widget::Point<int32_t>(x, y),
        .delta = widget::Point<int32_t>(x - prevX, y - prevY),
        .modifiers = modifiers,
        .timestamp = static_cast<uint64_t>(
            std::chrono::steady_clock::now().time_since_epoch().count()
        )
    };
}

// Create mouse button event
MouseButtonEvent createMouseButtonEvent(
    MouseButtonEvent::Button button,
    MouseButtonEvent::Action action,
    int32_t x, int32_t y,
    uint32_t modifiers
) {
    return MouseButtonEvent{
        .button = button,
        .action = action,
        .position = widget::Point<int32_t>(x, y),
        .modifiers = modifiers,
        .timestamp = static_cast<uint64_t>(
            std::chrono::steady_clock::now().time_since_epoch().count()
        )
    };
}

// Create key event
KeyEvent createKeyEvent(
    uint32_t keyCode,
    KeyEvent::Action action,
    uint32_t modifiers
) {
    return KeyEvent{
        .keyCode = keyCode,
        .action = action,
        .modifiers = modifiers,
        .timestamp = static_cast<uint64_t>(
            std::chrono::steady_clock::now().time_since_epoch().count()
        )
    };
}

// Create resize event
ResizeEvent createResizeEvent(
    uint32_t newWidth, uint32_t newHeight,
    uint32_t oldWidth, uint32_t oldHeight
) {
    return ResizeEvent{
        .newSize = widget::Size<uint32_t>(newWidth, newHeight),
        .oldSize = widget::Size<uint32_t>(oldWidth, oldHeight)
    };
}

// Create paint event
PaintEvent createPaintEvent(
    int32_t x, int32_t y,
    uint32_t width, uint32_t height
) {
    return PaintEvent{
        .dirtyRect = widget::Rect<int32_t, uint32_t>(x, y, width, height)
    };
}

// Helper: Check if event is input event (mouse/keyboard)
bool isInputEvent(const Event& event) {
    return std::holds_alternative<MouseMoveEvent>(event) ||
           std::holds_alternative<MouseButtonEvent>(event) ||
           std::holds_alternative<KeyEvent>(event);
}

// Helper: Check if event is window event (resize/paint)
bool isWindowEvent(const Event& event) {
    return std::holds_alternative<ResizeEvent>(event) ||
           std::holds_alternative<PaintEvent>(event);
}

// Helper: Get event type name (for debugging)
const char* getEventTypeName(const Event& event) {
    return std::visit([](auto&& arg) -> const char* {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::monostate>) return "None";
        else if constexpr (std::is_same_v<T, MouseMoveEvent>) return "MouseMove";
        else if constexpr (std::is_same_v<T, MouseButtonEvent>) return "MouseButton";
        else if constexpr (std::is_same_v<T, KeyEvent>) return "Key";
        else if constexpr (std::is_same_v<T, ResizeEvent>) return "Resize";
        else if constexpr (std::is_same_v<T, PaintEvent>) return "Paint";
        else if constexpr (std::is_same_v<T, FileDropEvent>) return "FileDrop";
        else return "Unknown";
    }, event);
}

} // namespace frqs::event