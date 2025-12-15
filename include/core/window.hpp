#pragma once

#include <memory>
#include <string>
#include "window_id.hpp"
#include "../unit/rect.hpp"
#include "../widget/iwidget.hpp"

namespace frqs::core {

// Forward declarations
class WindowRegistry;

// Forward declare unsafe namespace and its function BEFORE using as friend
namespace unsafe {
    void* getNativeHandle(const class Window* window) noexcept;
}

// ============================================================================
// WINDOW CREATION PARAMETERS
// ============================================================================

struct WindowParams {
    std::wstring title = L"FRQS Window";
    widget::Size<uint32_t> size {800, 600};
    widget::Point<int32_t> position {100, 100};
    bool resizable = true;
    bool visible = true;
    bool decorated = true;  // Title bar + borders
};

// ============================================================================
// WINDOW CLASS (PImpl Idiom - Hides HWND completely)
// ============================================================================

class Window {
private:
    struct Impl;  // Forward declare - defined in .cpp (hides windows.h)
    std::unique_ptr<Impl> pImpl_;
    WindowId id_;  // Now complete type

    // Private constructor - use create() factory
    explicit Window(const WindowParams& params);

public:
    ~Window() noexcept;

    // Factory method (returns shared_ptr for registry management)
    [[nodiscard]] static std::shared_ptr<Window> create(
        const WindowParams& params = WindowParams{}
    );

    // Non-copyable, non-movable (managed by shared_ptr)
    Window(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;

    // ========================================================================
    // PUBLIC API (No HWND exposure)
    // ========================================================================

    // Window properties
    void setTitle(const std::wstring& title);
    std::wstring getTitle() const;

    void setSize(const widget::Size<uint32_t>& size);
    widget::Size<uint32_t> getSize() const noexcept;

    void setPosition(const widget::Point<int32_t>& pos);
    widget::Point<int32_t> getPosition() const noexcept;

    widget::Rect<int32_t, uint32_t> getClientRect() const noexcept;

    // Visibility
    void show() noexcept;
    void hide() noexcept;
    bool isVisible() const noexcept;

    // Focus
    void setFocus() noexcept;
    bool hasFocus() const noexcept;

    // State
    void minimize() noexcept;
    void maximize() noexcept;
    void restore() noexcept;
    void close() noexcept;

    // Widget tree
    void setRootWidget(std::shared_ptr<widget::IWidget> root);
    std::shared_ptr<widget::IWidget> getRootWidget() const noexcept;

    // Rendering control
    void invalidate() noexcept;  // Full repaint
    void invalidateRect(const widget::Rect<int32_t, uint32_t>& rect) noexcept;
    void forceRedraw() noexcept;  // Immediate redraw

    // Window ID (for registry lookups)
    WindowId getId() const noexcept { return id_; }

    // ========================================================================
    // UNSAFE BACKDOOR (For advanced users ONLY)
    // ========================================================================
    friend class WindowRegistry;
    friend void* unsafe::getNativeHandle(const Window* window) noexcept;

private:
    void* getNativeHandleUnsafe() const noexcept;  // Returns HWND as void*
};

} // namespace frqs::core

// ============================================================================
// UNSAFE NAMESPACE (Explicit danger zone)
// ============================================================================

namespace frqs::core::unsafe {

// WARNING: This function breaks encapsulation!
// Use ONLY for platform-specific integration (e.g., OpenGL context creation)
// The returned handle becomes invalid if the window is destroyed.
inline void* getNativeHandle(const Window* window) noexcept {
    return window ? window->getNativeHandleUnsafe() : nullptr;
}

} // namespace frqs::core::unsafe