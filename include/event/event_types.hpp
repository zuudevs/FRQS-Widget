#pragma once

#include <cstdint>
#include <vector>
#include <filesystem>
#include "../unit/point.hpp"

namespace frqs::event {

// ============================================================================
// MODIFIER KEY FLAGS (Bitfield)
// ============================================================================

enum class ModifierKey : uint32_t {
    None      = 0,
    Shift     = 1 << 0,  // 0x01
    Control   = 1 << 1,  // 0x02
    Alt       = 1 << 2,  // 0x04
    Super     = 1 << 3,  // 0x08 (Windows key)
    CapsLock  = 1 << 4,  // 0x10
    NumLock   = 1 << 5,  // 0x20
};

// Bitwise operators for ModifierKey
inline constexpr ModifierKey operator|(ModifierKey lhs, ModifierKey rhs) noexcept {
    return static_cast<ModifierKey>(
        static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)
    );
}

inline constexpr ModifierKey operator&(ModifierKey lhs, ModifierKey rhs) noexcept {
    return static_cast<ModifierKey>(
        static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)
    );
}

inline constexpr ModifierKey operator^(ModifierKey lhs, ModifierKey rhs) noexcept {
    return static_cast<ModifierKey>(
        static_cast<uint32_t>(lhs) ^ static_cast<uint32_t>(rhs)
    );
}

inline constexpr ModifierKey operator~(ModifierKey key) noexcept {
    return static_cast<ModifierKey>(~static_cast<uint32_t>(key));
}

inline constexpr bool hasModifier(uint32_t modifiers, ModifierKey key) noexcept {
    return (modifiers & static_cast<uint32_t>(key)) != 0;
}

// ============================================================================
// VIRTUAL KEY CODES (Windows VK_* compatible)
// ============================================================================

enum class KeyCode : uint32_t {
    // Mouse buttons
    LButton    = 0x01,
    RButton    = 0x02,
    MButton    = 0x04,
    XButton1   = 0x05,
    XButton2   = 0x06,

    // Control keys
    Back       = 0x08,  // Backspace
    Tab        = 0x09,
    Return     = 0x0D,  // Enter
    Shift      = 0x10,
    Control    = 0x11,
    Alt        = 0x12,
    Pause      = 0x13,
    CapsLock   = 0x14,
    Escape     = 0x1B,
    Space      = 0x20,
    
    // Navigation
    PageUp     = 0x21,
    PageDown   = 0x22,
    End        = 0x23,
    Home       = 0x24,
    Left       = 0x25,
    Up         = 0x26,
    Right      = 0x27,
    Down       = 0x28,
    
    // Edit keys
    Insert     = 0x2D,
    Delete     = 0x2E,
    
    // Numbers (0-9)
    Key0       = 0x30,
    Key1       = 0x31,
    Key2       = 0x32,
    Key3       = 0x33,
    Key4       = 0x34,
    Key5       = 0x35,
    Key6       = 0x36,
    Key7       = 0x37,
    Key8       = 0x38,
    Key9       = 0x39,
    
    // Letters (A-Z)
    A          = 0x41,
    B          = 0x42,
    C          = 0x43,
    D          = 0x44,
    E          = 0x45,
    F          = 0x46,
    G          = 0x47,
    H          = 0x48,
    I          = 0x49,
    J          = 0x4A,
    K          = 0x4B,
    L          = 0x4C,
    M          = 0x4D,
    N          = 0x4E,
    O          = 0x4F,
    P          = 0x50,
    Q          = 0x51,
    R          = 0x52,
    S          = 0x53,
    T          = 0x54,
    U          = 0x55,
    V          = 0x56,
    W          = 0x57,
    X          = 0x58,
    Y          = 0x59,
    Z          = 0x5A,
    
    // Windows keys
    LWin       = 0x5B,
    RWin       = 0x5C,
    Apps       = 0x5D,  // Context menu
    
    // Numpad
    Numpad0    = 0x60,
    Numpad1    = 0x61,
    Numpad2    = 0x62,
    Numpad3    = 0x63,
    Numpad4    = 0x64,
    Numpad5    = 0x65,
    Numpad6    = 0x66,
    Numpad7    = 0x67,
    Numpad8    = 0x68,
    Numpad9    = 0x69,
    Multiply   = 0x6A,
    Add        = 0x6B,
    Separator  = 0x6C,
    Subtract   = 0x6D,
    Decimal    = 0x6E,
    Divide     = 0x6F,
    
    // Function keys
    F1         = 0x70,
    F2         = 0x71,
    F3         = 0x72,
    F4         = 0x73,
    F5         = 0x74,
    F6         = 0x75,
    F7         = 0x76,
    F8         = 0x77,
    F9         = 0x78,
    F10        = 0x79,
    F11        = 0x7A,
    F12        = 0x7B,
    
    // Lock keys
    NumLock    = 0x90,
    ScrollLock = 0x91,
    
    // Shift/Ctrl/Alt variants
    LShift     = 0xA0,
    RShift     = 0xA1,
    LControl   = 0xA2,
    RControl   = 0xA3,
    LAlt       = 0xA4,
    RAlt       = 0xA5,
};

// ============================================================================
// WINDOW EVENT TYPES
// ============================================================================

enum class WindowEventType : uint8_t {
    Created,
    Destroyed,
    Shown,
    Hidden,
    Minimized,
    Maximized,
    Restored,
    Focused,
    Unfocused,
    Moved,
    Resized,
    Closing,
    Closed,
};

// ============================================================================
// CURSOR TYPE
// ============================================================================

enum class CursorType : uint8_t {
    Arrow,
    IBeam,
    Wait,
    Crosshair,
    Hand,
    SizeNS,     // North-South resize
    SizeEW,     // East-West resize
    SizeNESW,   // Diagonal resize
    SizeNWSE,   // Diagonal resize
    SizeAll,    // Move
    No,         // Not allowed
    Custom,
};

// ============================================================================
// FILE DROP EVENT (NEW)
// ============================================================================

struct FileDropEvent {
    widget::Point<int32_t> position;
    std::vector<std::filesystem::path> files;
    
    FileDropEvent() = default;
    
    FileDropEvent(widget::Point<int32_t> pos, std::vector<std::filesystem::path> fileList)
        : position(pos), files(std::move(fileList)) {}
};

} // namespace frqs::event