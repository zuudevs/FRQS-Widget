/**
 * @file event_types.hpp
 * @brief Defines all core event data structures and enumerations for the event system.
 * @author zuudevs (zuudevs@gmail.com)
 * @version 0.1
 * @date 2025-12-24
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include <cstdint>
#include <vector>
#include <filesystem>
#include "unit/rect.hpp"

namespace frqs::event {

// ============================================================================
// MODIFIER KEY FLAGS (Bitfield)
// ============================================================================

/**
 * @enum ModifierKey
 * @brief A bitfield representing the state of modifier keys (Shift, Ctrl, etc.).
 */
enum class ModifierKey : uint32_t {
    None      = 0,         ///< No modifier keys are pressed.
    Shift     = 1 << 0,    ///< A Shift key is pressed.
    Control   = 1 << 1,    ///< A Control key is pressed.
    Alt       = 1 << 2,    ///< An Alt (Menu) key is pressed.
    Super     = 1 << 3,    ///< The Super key (Windows key or Command key) is pressed.
    CapsLock  = 1 << 4,    ///< Caps Lock is active.
    NumLock   = 1 << 5,    ///< Num Lock is active.
};

/// @brief Combines two ModifierKey flags.
inline constexpr ModifierKey operator|(ModifierKey lhs, ModifierKey rhs) noexcept {
    return static_cast<ModifierKey>(
        static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)
    );
}

/// @brief Checks for the presence of a ModifierKey flag in a bitmask.
inline constexpr ModifierKey operator&(ModifierKey lhs, ModifierKey rhs) noexcept {
    return static_cast<ModifierKey>(
        static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)
    );
}

/// @brief Toggles ModifierKey flags.
inline constexpr ModifierKey operator^(ModifierKey lhs, ModifierKey rhs) noexcept {
    return static_cast<ModifierKey>(
        static_cast<uint32_t>(lhs) ^ static_cast<uint32_t>(rhs)
    );
}

/// @brief Inverts a ModifierKey bitmask.
inline constexpr ModifierKey operator~(ModifierKey key) noexcept {
    return static_cast<ModifierKey>(~static_cast<uint32_t>(key));
}

/**
 * @brief Helper function to check if a specific modifier key is set in a bitfield.
 * @param modifiers The bitfield of active modifiers.
 * @param key The specific modifier key to check for.
 * @return `true` if the key is present in the bitfield, `false` otherwise.
 */
inline constexpr bool hasModifier(uint32_t modifiers, ModifierKey key) noexcept {
    return (modifiers & static_cast<uint32_t>(key)) != 0;
}

// ============================================================================
// VIRTUAL KEY CODES (Windows VK_* compatible)
// ============================================================================

/**
 * @enum KeyCode
 * @brief Platform-independent virtual key codes, compatible with Windows VK_* constants.
 */
enum class KeyCode : uint32_t {
    // Mouse buttons
    LButton    = 0x01, ///< Left mouse button
    RButton    = 0x02, ///< Right mouse button
    MButton    = 0x04, ///< Middle mouse button (wheel button)
    XButton1   = 0x05, ///< First X mouse button
    XButton2   = 0x06, ///< Second X mouse button

    // Control keys
    Back       = 0x08, ///< Backspace key
    Tab        = 0x09, ///< Tab key
    Return     = 0x0D, ///< Enter key
    Shift      = 0x10, ///< Shift key
    Control    = 0x11, ///< Ctrl key
    Alt        = 0x12, ///< Alt key
    Pause      = 0x13, ///< Pause key
    CapsLock   = 0x14, ///< Caps Lock key
    Escape     = 0x1B, ///< Esc key
    Space      = 0x20, ///< Spacebar

    // Navigation
    PageUp     = 0x21, ///< Page Up key
    PageDown   = 0x22, ///< Page Down key
    End        = 0x23, ///< End key
    Home       = 0x24, ///< Home key
    Left       = 0x25, ///< Left Arrow key
    Up         = 0x26, ///< Up Arrow key
    Right      = 0x27, ///< Right Arrow key
    Down       = 0x28, ///< Down Arrow key

    // Edit keys
    Insert     = 0x2D, ///< Insert key
    Delete     = 0x2E, ///< Delete key

    // Numbers (0-9)
    Key0 = 0x30, Key1, Key2, Key3, Key4, Key5, Key6, Key7, Key8, Key9,

    // Letters (A-Z)
    A = 0x41, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

    // Windows keys
    LWin       = 0x5B, ///< Left Windows key
    RWin       = 0x5C, ///< Right Windows key
    Apps       = 0x5D, ///< Applications key

    // Numpad
    Numpad0    = 0x60, Numpad1, Numpad2, Numpad3, Numpad4, Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
    Multiply   = 0x6A, ///< Numpad Multiply key
    Add        = 0x6B, ///< Numpad Add key
    Separator  = 0x6C, ///< Numpad Separator key
    Subtract   = 0x6D, ///< Numpad Subtract key
    Decimal    = 0x6E, ///< Numpad Decimal key
    Divide     = 0x6F, ///< Numpad Divide key

    // Function keys
    F1         = 0x70, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

    // Lock keys
    NumLock    = 0x90, ///< Num Lock key
    ScrollLock = 0x91, ///< Scroll Lock key

    // Shift/Ctrl/Alt variants
    LShift     = 0xA0, ///< Left Shift key
    RShift     = 0xA1, ///< Right Shift key
    LControl   = 0xA2, ///< Left Ctrl key
    RControl   = 0xA3, ///< Right Ctrl key
    LAlt       = 0xA4, ///< Left Alt key
    RAlt       = 0xA5, ///< Right Alt key
};

// ============================================================================
// WINDOW EVENT TYPES
// ============================================================================

/** @enum WindowEventType @brief Categorizes high-level window state changes. */
enum class WindowEventType : uint8_t {
    Created, Destroyed, Shown, Hidden, Minimized, Maximized,
    Restored, Focused, Unfocused, Moved, Resized, Closing, Closed,
};

// ============================================================================
// CURSOR TYPE
// ============================================================================

/** @enum CursorType @brief Defines standard system cursor styles. */
enum class CursorType : uint8_t {
    Arrow, IBeam, Wait, Crosshair, Hand, SizeNS, SizeEW,
    SizeNESW, SizeNWSE, SizeAll, No, Custom,
};

// ============================================================================
// EVENT STRUCT DEFINITIONS
// ============================================================================

/** @struct MouseMoveEvent @brief Sent when the mouse cursor moves. */
struct MouseMoveEvent {
    widget::Point<int32_t> position;  ///< Current cursor position in client coordinates.
    widget::Point<int32_t> delta;     ///< Change in position since the last move event.
    uint32_t modifiers;               ///< Bitfield of active `ModifierKey`s.
    uint64_t timestamp;               ///< High-precision timestamp of the event.
};

/** @struct MouseButtonEvent @brief Sent when a mouse button is pressed, released, or double-clicked. */
struct MouseButtonEvent {
    /** @enum Button @brief The mouse button that triggered the event. */
    enum class Button : uint8_t { Left, Right, Middle, X1, X2 };
    /** @enum Action @brief The action performed on the button. */
    enum class Action : uint8_t { Press, Release, DoubleClick };
    
    Button button;                    ///< The button that was acted upon.
    Action action;                    ///< The action that occurred.
    widget::Point<int32_t> position;  ///< Cursor position at the time of the event.
    uint32_t modifiers;               ///< Bitfield of active `ModifierKey`s.
    uint64_t timestamp;               ///< High-precision timestamp of the event.
};

/** @struct MouseWheelEvent @brief Sent when the mouse wheel is scrolled. */
struct MouseWheelEvent {
    int32_t delta;                    ///< The distance the wheel was rotated. Positive for forward, negative for backward.
    widget::Point<int32_t> position;  ///< Cursor position at the time of the event.
    uint32_t modifiers;               ///< Bitfield of active `ModifierKey`s.
    uint64_t timestamp;               ///< High-precision timestamp of the event.
};

/** @struct KeyEvent @brief Sent when a keyboard key is pressed, released, or held down. */
struct KeyEvent {
    /** @enum Action @brief The action performed on the key. */
    enum class Action : uint8_t { Press, Release, Repeat };
    
    uint32_t keyCode;                 ///< The `KeyCode` of the key.
    Action action;                    ///< The action that occurred.
    uint32_t modifiers;               ///< Bitfield of active `ModifierKey`s.
    uint64_t timestamp;               ///< High-precision timestamp of the event.
};

/** @struct ResizeEvent @brief Sent when the window's client area changes size. */
struct ResizeEvent {
    widget::Size<uint32_t> newSize;   ///< The new size of the window.
    widget::Size<uint32_t> oldSize;   ///< The size of the window before the resize.
};

/** @struct PaintEvent @brief Sent when a part of the window needs to be redrawn. */
struct PaintEvent {
    widget::Rect<int32_t, uint32_t> dirtyRect; ///< The rectangular area that needs repainting.
};

/** @struct FileDropEvent @brief Sent when one or more files are dropped onto the window. */
struct FileDropEvent {
    widget::Point<int32_t> position;            ///< The cursor position where the files were dropped.
    std::vector<std::filesystem::path> files;   ///< A list of paths to the dropped files.

    FileDropEvent() = default;
    
    /** @brief Constructs a FileDropEvent from a position and a list of paths. */
    FileDropEvent(widget::Point<int32_t> pos, std::vector<std::filesystem::path> fileList)
        : position(pos), files(std::move(fileList)) {}
    
    /** @brief Helper constructor to convert from a list of `std::wstring` paths. */
    FileDropEvent(widget::Point<int32_t> pos, const std::vector<std::wstring>& paths)
        : position(pos) {
        files.reserve(paths.size());
        for (const auto& p : paths) {
            files.emplace_back(p);
        }
    }
};

} // namespace frqs::event