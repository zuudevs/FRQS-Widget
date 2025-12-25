/**
 * @file event_dispatcher.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2025-12-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "event/event.hpp"
#include "widget/iwidget.hpp"
#include <chrono>

namespace frqs::event {

// ============================================================================
// EVENT DISPATCHER (Helper Functions)
// ============================================================================

/**
 * @brief Dispatches an event to the widget tree using a depth-first traversal.
 * 
 * This function recursively travels down the widget hierarchy. For each widget,
 * it first attempts to dispatch the event to its children (in front-to-back order
 * to respect z-ordering). If no child handles the event, it then offers the event
 * to the widget itself. The traversal stops and returns `true` as soon as a widget
 * in the tree handles the event.
 * 
 * @param root The root widget of the tree (or subtree) to dispatch the event to.
 * @param event The event to be dispatched.
 * @return `true` if the event was handled by any widget in the tree, `false` otherwise.
 */
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

/**
 * @brief Creates a MouseMoveEvent from native coordinate data.
 * 
 * @param x The new x-coordinate of the mouse.
 * @param y The new y-coordinate of the mouse.
 * @param prevX The previous x-coordinate of the mouse.
 * @param prevY The previous y-coordinate of the mouse.
 * @param modifiers A bitmask of modifier keys (e.g., Shift, Ctrl) held down during the event.
 * @return A fully constructed MouseMoveEvent object.
 */
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

/**
 * @brief Creates a MouseButtonEvent.
 * 
 * @param button The mouse button associated with the event (e.g., Left, Right).
 * @param action The action performed (e.g., Press, Release).
 * @param x The x-coordinate of the mouse cursor when the event occurred.
 * @param y The y-coordinate of the mouse cursor when the event occurred.
 * @param modifiers A bitmask of modifier keys held down.
 * @return A fully constructed MouseButtonEvent object.
 */
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

/**
 * @brief Creates a KeyEvent.
 * 
 * @param keyCode The virtual key code of the key.
 * @param action The action performed (e.g., Press, Release, Repeat).
 * @param modifiers A bitmask of modifier keys held down.
 * @return A fully constructed KeyEvent object.
 */
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

/**
 * @brief Creates a ResizeEvent.
 * 
 * @param newWidth The new width of the window or area.
 * @param newHeight The new height of the window or area.
 * @param oldWidth The previous width.
 * @param oldHeight The previous height.
 * @return A fully constructed ResizeEvent object.
 */
ResizeEvent createResizeEvent(
    uint32_t newWidth, uint32_t newHeight,
    uint32_t oldWidth, uint32_t oldHeight
) {
    return ResizeEvent{
        .newSize = widget::Size<uint32_t>(newWidth, newHeight),
        .oldSize = widget::Size<uint32_t>(oldWidth, oldHeight)
    };
}

/**
 * @brief Creates a PaintEvent.
 * 
 * @param x The x-coordinate of the top-left corner of the dirty rectangle.
 * @param y The y-coordinate of the top-left corner of the dirty rectangle.
 * @param width The width of the dirty rectangle.
 * @param height The height of the dirty rectangle.
 * @return A fully constructed PaintEvent object.
 */
PaintEvent createPaintEvent(
    int32_t x, int32_t y,
    uint32_t width, uint32_t height
) {
    return PaintEvent{
        .dirtyRect = widget::Rect<int32_t, uint32_t>(x, y, width, height)
    };
}

/**
 * @brief Checks if an event is an input event (mouse or keyboard).
 * 
 * @param event The event to check.
 * @return `true` if the event is a MouseMoveEvent, MouseButtonEvent, or KeyEvent, `false` otherwise.
 */
bool isInputEvent(const Event& event) {
    return std::holds_alternative<MouseMoveEvent>(event) ||
           std::holds_alternative<MouseButtonEvent>(event) ||
           std::holds_alternative<KeyEvent>(event);
}

/**
 * @brief Checks if an event is a window management event.
 * 
 * @param event The event to check.
 * @return `true` if the event is a ResizeEvent or PaintEvent, `false` otherwise.
 */
bool isWindowEvent(const Event& event) {
    return std::holds_alternative<ResizeEvent>(event) ||
           std::holds_alternative<PaintEvent>(event);
}

/**
 * @brief Gets the type name of an event as a string literal.
 * 
 * This is primarily a helper for debugging and logging purposes.
 * 
 * @param event The event whose type name is to be retrieved.
 * @return A C-style string representing the event's type (e.g., "MouseMove", "KeyEvent").
 */
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