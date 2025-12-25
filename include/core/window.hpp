/**
 * @file window.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2025-12-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <memory>
#include <string>
#include "window_id.hpp"
#include "unit/rect.hpp"
#include "widget/iwidget.hpp"
#include "event/event.hpp"

// Forward declare to avoid including heavy platform-specific headers.
namespace frqs::platform {
    class Win32WindowClass;
}

namespace frqs::core {

// Forward declarations
class WindowRegistry;
class Window;

// Forward declare the unsafe namespace and its function before using it as a friend.
namespace unsafe {
    void* getNativeHandle(const class Window* window) noexcept;
}

// ============================================================================
// WINDOW CREATION PARAMETERS
// ============================================================================

/**
 * @struct WindowParams
 * @brief A struct to specify the initial properties of a new window.
 */
struct WindowParams {
    /** @brief The text displayed in the window's title bar. */
    std::wstring title = L"FRQS Window";
    /** @brief The initial dimensions (width and height) of the window's client area. */
    widget::Size<uint32_t> size {800, 600};
    /** @brief The initial screen coordinates of the window's top-left corner. */
    widget::Point<int32_t> position {100, 100};
    /** @brief If `true`, the user can resize the window. */
    bool resizable = true;
    /** @brief If `true`, the window is made visible immediately after creation. */
    bool visible = true;
    /** @brief If `true`, the window has a standard title bar, border, and system menu. */
    bool decorated = true;
};

// ============================================================================
// WINDOW CLASS (PImpl Idiom - Hides HWND completely)
// ============================================================================

/**
 * @class Window
 * @brief An abstraction over a native operating system window.
 *
 * This class manages the lifecycle, properties, and content of a window.
 * It uses the PImpl (pointer to implementation) idiom to hide all platform-specific
 * details (like the native HWND on Windows) from the public interface.
 *
 * Windows are created via the static `create()` factory method, which returns
 * a `std::shared_ptr`. This allows them to be managed by the `WindowRegistry`.
 * The class is non-copyable and non-movable to enforce this shared ownership model.
 */
class Window {
private:
    /// @brief Forward-declared private implementation (defined in .cpp).
    struct Impl;
    /// @brief Pointer to the private implementation.
    std::unique_ptr<Impl> pImpl_;
    /// @brief The unique identifier for this window, assigned by the registry.
    WindowId id_;

    /**
     * @brief Private constructor to enforce creation via the `create()` factory.
     * @param params The initial parameters for the window.
     */
    explicit Window(const WindowParams& params);

public:
    /**
     * @brief Destructor. Handles the cleanup and destruction of the native window.
     */
    ~Window() noexcept;

    /**
     * @brief Factory method to create, initialize, and register a new window.
     * @param params The initial parameters for the window.
     * @return A `std::shared_ptr<Window>` to the newly created window.
     */
    [[nodiscard]] static std::shared_ptr<Window> create(
        const WindowParams& params = WindowParams{}
    );

    // Windows are managed by shared_ptr and the WindowRegistry, so copying and
    // moving are deleted to prevent ownership confusion.
    Window(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;

    // ========================================================================
    // PUBLIC API (Platform-Independent)
    // ========================================================================

    // --- Window Properties ---

    /** @brief Sets the text of the window's title bar. */
    void setTitle(const std::wstring& title);
    /** @brief Gets the current text of the window's title bar. */
    std::wstring getTitle() const;

    /** @brief Sets the size of the window's client area. */
    void setSize(const widget::Size<uint32_t>& size);
    /** @brief Gets the current size of the window's client area. */
    widget::Size<uint32_t> getSize() const noexcept;

    /** @brief Sets the screen position of the window's top-left corner. */
    void setPosition(const widget::Point<int32_t>& pos);
    /** @brief Gets the screen position of the window's top-left corner. */
    widget::Point<int32_t> getPosition() const noexcept;

    /** @brief Gets the rectangle defining the window's client area, with position (0,0). */
    widget::Rect<int32_t, uint32_t> getClientRect() const noexcept;

    // --- Visibility ---

    /** @brief Shows the window if it is hidden. */
    void show() noexcept;
    /** @brief Hides the window if it is visible. */
    void hide() noexcept;
    /** @brief Checks if the window is currently visible. */
    bool isVisible() const noexcept;

    // --- Focus ---

    /** @brief Attempts to give the window input focus. */
    void setFocus() noexcept;
    /** @brief Checks if the window currently has input focus. */
    bool hasFocus() const noexcept;

    // --- State ---

    /** @brief Minimizes the window. */
    void minimize() noexcept;
    /** @brief Maximizes the window. */
    void maximize() noexcept;
    /** @brief Restores the window to its normal state from minimized or maximized. */
    void restore() noexcept;
    /** @brief Closes the window, beginning the destruction process. */
    void close() noexcept;

    // --- Widget Tree ---

    /** @brief Sets the root widget that will be displayed in this window. */
    void setRootWidget(std::shared_ptr<widget::IWidget> root);
    /** @brief Gets the current root widget of the window. */
    std::shared_ptr<widget::IWidget> getRootWidget() const noexcept;

    // --- Rendering Control ---

    /** @brief Invalidates the entire window, queueing a full repaint for the next frame. */
    void invalidate() noexcept;
    /** @brief Invalidates a specific rectangular area of the window for repainting. */
    void invalidateRect(const widget::Rect<int32_t, uint32_t>& rect) noexcept;
    /** @brief Forces an immediate, synchronous redraw of the window's invalid regions. */
    void forceRedraw() noexcept;
	
    /**
     * @brief Dispatches an event to the window's widget hierarchy.
     * @param event The event to dispatch.
     */
	void dispatchEvent(const event::Event& event);

    /**
     * @brief Gets the window's unique identifier.
     * @return The `WindowId` assigned by the `WindowRegistry`.
     */
    WindowId getId() const noexcept { return id_; }

    // ========================================================================
    // FRIEND DECLARATIONS & UNSAFE BACKDOOR
    // ========================================================================

    /// @brief Allows the registry to manage window lifecycle.
    friend class WindowRegistry;
    /// @brief Allows the unsafe function to access the private implementation.
    friend void* unsafe::getNativeHandle(const Window* window) noexcept;
    /// @brief Allows the platform-specific window class to access the private implementation.
	friend class frqs::platform::Win32WindowClass;

private:
    /**
     * @brief Retrieves the native window handle as a `void*`.
     * @return The native handle (e.g., HWND) cast to `void*`.
     * @internal This is the private implementation of the unsafe backdoor.
     */
    void* getNativeHandleUnsafe() const noexcept;
};

} // namespace frqs::core

// ============================================================================
// UNSAFE NAMESPACE (Explicit danger zone)
// ============================================================================

/**
 * @namespace frqs::core::unsafe
 * @brief Contains functions that break encapsulation for platform integration.
 *
 * Functions in this namespace are inherently dangerous and should be used with
 * extreme caution. They provide access to underlying native handles, which

 * couples the calling code to a specific platform.
 */
namespace frqs::core::unsafe {

/**
 * @brief Retrieves the raw, native handle of a window.
 * @warning This function breaks the primary abstraction of the `Window` class.
 *          Use it ONLY for essential platform-specific integration that cannot
 *          be done through the public API (eg., creating a graphics context
 *          like OpenGL or Vulkan). The returned handle becomes invalid if the
 *          window is destroyed.
 * @param window A pointer to the `Window` object.
 * @return The native handle (e.g., HWND on Windows) as a `void*`, or `nullptr`
 *         if the window pointer is null.
 */
inline void* getNativeHandle(const Window* window) noexcept {
    return window ? window->getNativeHandleUnsafe() : nullptr;
}

} // namespace frqs::core::unsafe