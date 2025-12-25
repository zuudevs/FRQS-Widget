/**
 * @file window.cpp
 * @brief Implements the Window class, which abstracts a native OS window.
 * @author zuudevs (zuudevs@gmail.com)
 * @version 0.1
 * @date 2025-12-24
 *
 * @copyright Copyright (c) 2025
 *
 * This file contains the implementation for the platform-independent `Window`
 * class. It uses the PImpl idiom, delegating all platform-specific calls
 * (mostly Win32 API functions) to the `Window::Impl` struct.
 */

#include "widget/internal.hpp"
#include "window_impl.hpp"
#include "core/window_registry.hpp"

#ifdef _DEBUG
#include <print>
#endif

// Forward declaration of the platform-specific window creation function.
namespace frqs::platform {
    HWND createNativeWindow(const core::WindowParams& params, void* windowPtr);
}

// Forward declaration of internal widget function to link widget to a window.
namespace frqs::widget::internal {
    void setWidgetWindowHandle(Widget* widget, void* hwnd);
}

namespace frqs::core {

// ============================================================================
// HELPER: Calculate actual border size from window
// ============================================================================

/**
 * @brief Calculates the size of a window's non-client area (borders, title bar).
 * @param hwnd Handle to the native window.
 * @param[out] borderWidth The calculated total width of the vertical borders.
 * @param[out] borderHeight The calculated total height of the horizontal borders and title bar.
 * @internal
 */
static void getActualBorderSize(HWND hwnd, int& borderWidth, int& borderHeight) {
    RECT windowRect, clientRect;
    GetWindowRect(hwnd, &windowRect);
    GetClientRect(hwnd, &clientRect);

    // GetClientRect returns a rect relative to the client area's top-left.
    // GetWindowRect returns a rect in screen coordinates.
    // To compare them, we must convert the client rect to screen coordinates.
    POINT pt = {0, 0};
    ClientToScreen(hwnd, &pt);

    // The difference between the screen-based window rect and the screen-based
    // client rect gives us the size of the decorations.
    borderWidth = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
    borderHeight = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);
}

// ============================================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================================

Window::Window(const WindowParams& params)
    : pImpl_(std::make_unique<Impl>())
    , id_{0} // ID is initialized to 0 and set by the factory after registration.
{
    // Copy parameters from the creation struct to our private implementation.
    pImpl_->title = params.title;
    pImpl_->size = params.size;
    pImpl_->position = params.position;
    pImpl_->resizable = params.resizable;
    pImpl_->visible = params.visible;
    pImpl_->decorated = params.decorated;

    pImpl_->initializeDirtyRects();

    // Delegate to the platform-specific function to create the native window.
    // `this` is passed so the native window procedure can link back to this object.
    try {
        pImpl_->hwnd = platform::createNativeWindow(params, this);
        // Once the native handle is created, we can initialize the renderer.
        pImpl_->initializeRenderer();
    } catch (const std::exception& e) {
        // Wrap platform exceptions in a standard runtime_error.
        throw std::runtime_error(
            std::string("Failed to create window: ") + e.what()
        );
    }
}

Window::~Window() noexcept {
    // If the native window handle still exists, explicitly destroy it.
    // This is a safety measure; `close()` should normally be called.
    if (pImpl_->hwnd) {
        DestroyWindow(pImpl_->hwnd);
    }
}

// ============================================================================
// FACTORY METHOD
// ============================================================================

std::shared_ptr<Window> Window::create(const WindowParams& params) {
    // The constructor is private, so we use `new Window` and wrap it in a shared_ptr.
    // This ensures the object is created on the heap and its lifetime is managed.
    auto window = std::shared_ptr<Window>(new Window(params));

    // Register the newly created window with the singleton registry.
    // The registry assigns the final unique ID to the window.
    auto& registry = WindowRegistry::instance();
    window->id_ = registry.registerWindow(window);
    return window;
}

// ============================================================================
// WINDOW PROPERTIES
// ============================================================================

void Window::setTitle(const std::wstring& title) {
    pImpl_->title = title;
    if (pImpl_->hwnd) {
        SetWindowTextW(pImpl_->hwnd, title.c_str());
    }
}

std::wstring Window::getTitle() const {
    return pImpl_->title;
}

void Window::setSize(const widget::Size<uint32_t>& size) {
    // If the native window doesn't exist yet, just update the internal size.
    if (!pImpl_->hwnd) {
        pImpl_->size = size;
        pImpl_->updateDirtyRectBounds();
        if (pImpl_->rootWidget) {
            pImpl_->rootWidget->setRect(getClientRect());
        }
        return;
    }

    #ifdef _DEBUG
    std::println("=== setSize({}, {}) ===", size.w, size.h);
    #endif

    // Ensure the window is in a normal state before resizing.
    if (IsIconic(pImpl_->hwnd)) ShowWindow(pImpl_->hwnd, SW_RESTORE);
    if (IsZoomed(pImpl_->hwnd)) ShowWindow(pImpl_->hwnd, SW_RESTORE);
    
    // Process any pending messages before resizing to get an accurate state.
    MSG msg;
    while (PeekMessageW(&msg, pImpl_->hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    
    DWORD style = GetWindowLongW(pImpl_->hwnd, GWL_STYLE);
    DWORD exStyle = GetWindowLongW(pImpl_->hwnd, GWL_EXSTYLE);
    
    // We want to set the client area size. The OS sets the total window size.
    // `AdjustWindowRectExForDpi` calculates the required total window size
    // to achieve the desired client area size.
    RECT rect = {0, 0, static_cast<LONG>(size.w), static_cast<LONG>(size.h)};
    UINT dpi = GetDpiForWindow(pImpl_->hwnd);
    
    if (dpi > 0) {
        AdjustWindowRectExForDpi(&rect, style, FALSE, exStyle, dpi);
    } else {
        AdjustWindowRectEx(&rect, style, FALSE, exStyle);
    }
    
    int targetWindowWidth = rect.right - rect.left;
    int targetWindowHeight = rect.bottom - rect.top;
    
    // SWP_NOSENDCHANGING is a key flag to bypass OS-level size constraints
    // (from WM_GETMINMAXINFO) that can interfere with programmatic resizing.
    SetWindowPos(
        pImpl_->hwnd, 
        nullptr, 
        0, 0, // x, y (not changed)
        targetWindowWidth, 
        targetWindowHeight,
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING
    );
    
    UpdateWindow(pImpl_->hwnd); // Force a repaint.
    
    // Process messages again to handle the effects of SetWindowPos.
    while (PeekMessageW(&msg, pImpl_->hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    
    // Verify the actual size and update our internal state.
    RECT clientRect;
    GetClientRect(pImpl_->hwnd, &clientRect);
    int actualWidth = clientRect.right - clientRect.left;
    int actualHeight = clientRect.bottom - clientRect.top;
    
    pImpl_->handleSizeMessage(actualWidth, actualHeight);
}

widget::Size<uint32_t> Window::getSize() const noexcept {
    return pImpl_->size;
}

void Window::setPosition(const widget::Point<int32_t>& pos) {
    if (pImpl_->position == pos) return;
    
    pImpl_->position = pos;
    
    if (pImpl_->hwnd) {
        // Setting the window position is tricky due to borders, especially
        // the invisible DWM borders. We calculate the offset between the
        // window's top-left corner and the client area's top-left corner.
        RECT windowRect;
        GetWindowRect(pImpl_->hwnd, &windowRect);
        
        POINT clientTopLeft = {0, 0};
        ClientToScreen(pImpl_->hwnd, &clientTopLeft);
        
        int offsetX = clientTopLeft.x - windowRect.left;
        int offsetY = clientTopLeft.y - windowRect.top;
        
        // We adjust the target position by this offset to ensure the
        // *client area* ends up at the desired coordinates.
        int adjustedX = pos.x - offsetX;
        int adjustedY = pos.y - offsetY;
        
        SetWindowPos(pImpl_->hwnd, nullptr, adjustedX, adjustedY, 0, 0,
                     SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        
        // After setting, we re-query the actual position to keep our state accurate.
        GetWindowRect(pImpl_->hwnd, &windowRect);
        ClientToScreen(pImpl_->hwnd, &clientTopLeft);
        
        pImpl_->position.x = clientTopLeft.x;
        pImpl_->position.y = clientTopLeft.y;
    }
}

widget::Point<int32_t> Window::getPosition() const noexcept {
    return pImpl_->position;
}

widget::Rect<int32_t, uint32_t> Window::getClientRect() const noexcept {
    // The client rect is always relative to the window itself, so its top-left is (0,0).
    return widget::Rect<int32_t, uint32_t>(0, 0, pImpl_->size.w, pImpl_->size.h);
}

// ============================================================================
// VISIBILITY
// ============================================================================

void Window::show() noexcept {
    if (pImpl_->visible) return;
    pImpl_->visible = true;
    if (pImpl_->hwnd) {
        ShowWindow(pImpl_->hwnd, SW_SHOW);
        UpdateWindow(pImpl_->hwnd); // Triggers a WM_PAINT message.
    }
}

void Window::hide() noexcept {
    if (!pImpl_->visible) return;
    pImpl_->visible = false;
    if (pImpl_->hwnd) {
        ShowWindow(pImpl_->hwnd, SW_HIDE);
    }
}

bool Window::isVisible() const noexcept {
    return pImpl_->visible;
}

// ============================================================================
// FOCUS
// ============================================================================

void Window::setFocus() noexcept {
    if (pImpl_->hwnd) {
        // SetForegroundWindow brings the entire window to the front.
        SetForegroundWindow(pImpl_->hwnd);
        // SetFocus gives it keyboard input focus.
        ::SetFocus(pImpl_->hwnd);
    }
}

bool Window::hasFocus() const noexcept {
    return pImpl_->focused;
}

// ============================================================================
// STATE
// ============================================================================

void Window::minimize() noexcept {
    pImpl_->minimized = true;
    pImpl_->maximized = false;
    if (pImpl_->hwnd) {
        ShowWindow(pImpl_->hwnd, SW_MINIMIZE);
    }
}

void Window::maximize() noexcept {
    pImpl_->maximized = true;
    pImpl_->minimized = false;
    if (pImpl_->hwnd) {
        ShowWindow(pImpl_->hwnd, SW_MAXIMIZE);
    }
}

void Window::restore() noexcept {
    pImpl_->minimized = false;
    pImpl_->maximized = false;
    if (pImpl_->hwnd) {
        ShowWindow(pImpl_->hwnd, SW_RESTORE);
    }
}

void Window::close() noexcept {
    if (pImpl_->closed) return;
    pImpl_->closed = true;
    
    // The WM_CLOSE message in the window procedure will handle destroying the window.
    // Here we just need to initiate that process and unregister from the registry.
    if (pImpl_->hwnd) {
        // DestroyWindow will eventually lead to WM_NCDESTROY, where we nullify pImpl->hwnd.
        DestroyWindow(pImpl_->hwnd);
    }
    WindowRegistry::instance().unregisterWindow(id_);
}

// ============================================================================
// WIDGET TREE
// ============================================================================

void Window::setRootWidget(std::shared_ptr<widget::IWidget> root) {
    pImpl_->rootWidget = std::move(root);
    if (pImpl_->rootWidget) {
        // The root widget always occupies the entire client area of the window.
        pImpl_->rootWidget->setRect(getClientRect());
        
        // Give the widget tree a handle to this window so it can post invalidation requests.
        if (auto* rootAsWidget = dynamic_cast<widget::Widget*>(pImpl_->rootWidget.get())) {
            widget::internal::setWidgetWindowHandle(rootAsWidget, pImpl_->hwnd);
        }
        
        invalidate(); // Request a full repaint to draw the new widget.
    }
}

std::shared_ptr<widget::IWidget> Window::getRootWidget() const noexcept {
    return pImpl_->rootWidget;
}

// ============================================================================
// RENDERING CONTROL
// ============================================================================

void Window::invalidate() noexcept {
    if (pImpl_->dirtyRects) {
        pImpl_->dirtyRects->markFullRedraw();
    }
    // InvalidateRect tells the OS that the window's contents are invalid
    // and a WM_PAINT message should be sent when the application is idle.
    if (pImpl_->hwnd) {
        InvalidateRect(pImpl_->hwnd, nullptr, FALSE);
    }
}

void Window::invalidateRect(const widget::Rect<int32_t, uint32_t>& rect) noexcept {
    if (pImpl_->dirtyRects) {
        pImpl_->dirtyRects->addDirtyRect(rect);
    }
    if (pImpl_->hwnd) {
        RECT r = {
            static_cast<LONG>(rect.x),
            static_cast<LONG>(rect.y),
            static_cast<LONG>(rect.getRight()),
            static_cast<LONG>(rect.getBottom())
        };
        InvalidateRect(pImpl_->hwnd, &r, FALSE);
    }
}

void Window::forceRedraw() noexcept {
    // Bypasses the OS message queue and renders immediately.
    // Useful for animations or responsive feedback.
    if (pImpl_->renderer && pImpl_->rootWidget && pImpl_->visible) {
        pImpl_->render();
    }
}

// ============================================================================
// EVENT DISPATCH & UNSAFE BACKDOOR
// ============================================================================

void* Window::getNativeHandleUnsafe() const noexcept {
    return pImpl_->hwnd;
}

void Window::dispatchEvent(const event::Event& event) {
    if (!pImpl_->rootWidget) return;
    
    // --- MOUSE & FILE DROP EVENTS: Use hit-testing to find the target widget ---
    // Events with positional data are dispatched to the top-most widget under the cursor.
    auto processPositionalEvent = [&](const widget::Point<int32_t>& pos) {
        auto* target = pImpl_->rootWidget->hitTest(pos);
        if (target) {
            // Attempt to handle the event at the target. If it's not handled,
            // bubble the event up the widget hierarchy to its parents.
            for (auto* current = target; current != nullptr; current = current->getParent()) {
                if (current->onEvent(event)) return;
            }
        }
    };

    if (auto* mouseMove = std::get_if<event::MouseMoveEvent>(&event)) {
        processPositionalEvent(mouseMove->position);
        return;
    }
    if (auto* mouseButton = std::get_if<event::MouseButtonEvent>(&event)) {
        processPositionalEvent(mouseButton->position);
        return;
    }
    if (auto* mouseWheel = std::get_if<event::MouseWheelEvent>(&event)) {
        processPositionalEvent(mouseWheel->position);
        return;
    }
    if (auto* fileDrop = std::get_if<event::FileDropEvent>(&event)) {
        processPositionalEvent(fileDrop->position);
        return;
    }
    
    // --- KEYBOARD EVENTS: Send to the focused widget ---
    if (std::holds_alternative<event::KeyEvent>(event)) {
        // TODO: Implement proper focus management to find the correct target.
        // For now, all keyboard events are sent to the root widget.
        pImpl_->rootWidget->onEvent(event);
        return;
    }
    
    // --- WINDOW-WIDE EVENTS: Broadcast to the entire tree ---
    // Events like Resize or Paint apply to all widgets, so we broadcast them
    // by calling onEvent on the root, which should then propagate to children.
    if (std::holds_alternative<event::ResizeEvent>(event) ||
        std::holds_alternative<event::PaintEvent>(event)) {
        pImpl_->rootWidget->onEvent(event);
        return;
    }
    
    // For any other event type, we just send it to the root as a default action.
    pImpl_->rootWidget->onEvent(event);
}

} // namespace frqs::core