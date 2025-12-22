#include "widget/internal.hpp"
#include "window_impl.hpp"
#include "core/window_registry.hpp"

#ifdef _DEBUG
#include <print>
#endif

namespace frqs::platform {
    HWND createNativeWindow(const core::WindowParams& params, void* windowPtr);
}

namespace frqs::widget::internal {
    void setWidgetWindowHandle(Widget* widget, void* hwnd);
}

namespace frqs::core {

// ============================================================================
// HELPER: Calculate actual border size from window
// ============================================================================

static void getActualBorderSize(HWND hwnd, int& borderWidth, int& borderHeight) {
    RECT windowRect, clientRect;
    GetWindowRect(hwnd, &windowRect);
    GetClientRect(hwnd, &clientRect);
    
    // Convert client rect to screen coordinates
    POINT pt = {0, 0};
    ClientToScreen(hwnd, &pt);
    
    // Calculate total border size
    borderWidth = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
    borderHeight = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);
}

// ============================================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================================

Window::Window(const WindowParams& params)
    : pImpl_(std::make_unique<Impl>())
    , id_{0}
{
    pImpl_->title = params.title;
    pImpl_->size = params.size;
    pImpl_->position = params.position;
    pImpl_->resizable = params.resizable;
    pImpl_->visible = params.visible;
    pImpl_->decorated = params.decorated;

    pImpl_->initializeDirtyRects();

    try {
        pImpl_->hwnd = platform::createNativeWindow(params, this);
        pImpl_->initializeRenderer();
    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("Failed to create window: ") + e.what()
        );
    }
}

Window::~Window() noexcept {
    if (pImpl_->hwnd) {
        DestroyWindow(pImpl_->hwnd);
    }
}

// ============================================================================
// FACTORY METHOD
// ============================================================================

std::shared_ptr<Window> Window::create(const WindowParams& params) {
    auto window = std::shared_ptr<Window>(new Window(params));
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

    // Restore window if needed
    if (IsIconic(pImpl_->hwnd)) ShowWindow(pImpl_->hwnd, SW_RESTORE);
    if (IsZoomed(pImpl_->hwnd)) ShowWindow(pImpl_->hwnd, SW_RESTORE);
    
    MSG msg;
    while (PeekMessageW(&msg, pImpl_->hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    
    // Get current style
    DWORD style = GetWindowLongW(pImpl_->hwnd, GWL_STYLE);
    DWORD exStyle = GetWindowLongW(pImpl_->hwnd, GWL_EXSTYLE);
    
    // Use DPI-aware border calculation
    RECT rect = {0, 0, static_cast<LONG>(size.w), static_cast<LONG>(size.h)};
    UINT dpi = GetDpiForWindow(pImpl_->hwnd);
    
    if (dpi > 0) {
        AdjustWindowRectExForDpi(&rect, style, FALSE, exStyle, dpi);
    } else {
        AdjustWindowRectEx(&rect, style, FALSE, exStyle);
    }
    
    int targetWindowWidth = rect.right - rect.left;
    int targetWindowHeight = rect.bottom - rect.top;
    
    #ifdef _DEBUG
    std::println("Target window size: {}x{}", targetWindowWidth, targetWindowHeight);
    #endif
    
    // ✅ KEY FIX: Use SWP_NOSENDCHANGING to bypass WM_GETMINMAXINFO constraints
    SetWindowPos(
        pImpl_->hwnd, 
        nullptr, 
        0, 0,
        targetWindowWidth, 
        targetWindowHeight,
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING
    );
    
    UpdateWindow(pImpl_->hwnd);
    
    // Process all messages
    while (PeekMessageW(&msg, pImpl_->hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    
    // Verify
    RECT clientRect;
    GetClientRect(pImpl_->hwnd, &clientRect);
    int actualWidth = clientRect.right - clientRect.left;
    int actualHeight = clientRect.bottom - clientRect.top;
    
    #ifdef _DEBUG
    std::println("Result: client={}x{}, error={}x{}",
                actualWidth, actualHeight,
                static_cast<int>(size.w) - actualWidth,
                static_cast<int>(size.h) - actualHeight);
    #endif
    
    // Final state update
    pImpl_->handleSizeMessage(actualWidth, actualHeight);
    
    #ifdef _DEBUG
    std::println("=== Final size: {}x{} ===\n", pImpl_->size.w, pImpl_->size.h);
    #endif
}

widget::Size<uint32_t> Window::getSize() const noexcept {
    return pImpl_->size;
}

void Window::setPosition(const widget::Point<int32_t>& pos) {
    if (pImpl_->position == pos) return;
    
    pImpl_->position = pos;
    
    if (pImpl_->hwnd) {
        // Get window and client rects to calculate border offset
        RECT windowRect, clientRect;
        GetWindowRect(pImpl_->hwnd, &windowRect);
        
        // Convert client rect to screen coordinates
        clientRect.left = clientRect.top = 0;
        clientRect.right = pImpl_->size.w;
        clientRect.bottom = pImpl_->size.h;
        ClientToScreen(pImpl_->hwnd, reinterpret_cast<POINT*>(&clientRect));
        
        // Calculate the offset between window position and client position
        // (This accounts for invisible DWM borders)
        int offsetX = clientRect.left - windowRect.left;
        int offsetY = clientRect.top - windowRect.top;
        
        // Adjust target position to compensate for border offset
        int adjustedX = pos.x - offsetX;
        int adjustedY = pos.y - offsetY;
        
        #ifdef _DEBUG
        std::println("setPosition: target=({}, {}), offset=({}, {}), adjusted=({}, {})",
                    pos.x, pos.y, offsetX, offsetY, adjustedX, adjustedY);
        #endif
        
        SetWindowPos(pImpl_->hwnd, nullptr, adjustedX, adjustedY, 0, 0,
                     SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        
        // Verify actual position
        GetWindowRect(pImpl_->hwnd, &windowRect);
        POINT clientTopLeft = {0, 0};
        ClientToScreen(pImpl_->hwnd, &clientTopLeft);
        
        // Update stored position to actual client position
        pImpl_->position.x = clientTopLeft.x;
        pImpl_->position.y = clientTopLeft.y;
        
        #ifdef _DEBUG
        std::println("  Actual position: ({}, {})", pImpl_->position.x, pImpl_->position.y);
        #endif
    }
}

widget::Point<int32_t> Window::getPosition() const noexcept {
    return pImpl_->position;
}

widget::Rect<int32_t, uint32_t> Window::getClientRect() const noexcept {
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
        UpdateWindow(pImpl_->hwnd);
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
        SetForegroundWindow(pImpl_->hwnd);
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
    if (pImpl_->hwnd) {
        DestroyWindow(pImpl_->hwnd);
        pImpl_->hwnd = nullptr;
    }
    WindowRegistry::instance().unregisterWindow(id_);
}

// ============================================================================
// WIDGET TREE
// ============================================================================

void Window::setRootWidget(std::shared_ptr<widget::IWidget> root) {
    pImpl_->rootWidget = std::move(root);
    if (pImpl_->rootWidget) {
        pImpl_->rootWidget->setRect(getClientRect());
        
        // Set window handle for proper invalidation
        if (auto* rootAsWidget = dynamic_cast<widget::Widget*>(pImpl_->rootWidget.get())) {
            widget::internal::setWidgetWindowHandle(rootAsWidget, pImpl_->hwnd);
        }
        
        invalidate();
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
    if (pImpl_->renderer && pImpl_->rootWidget && pImpl_->visible) {
        pImpl_->render();
    }
}

// ============================================================================
// UNSAFE BACKDOOR
// ============================================================================

void* Window::getNativeHandleUnsafe() const noexcept {
    return pImpl_->hwnd;
}

void Window::dispatchEvent(const event::Event& event) {
    if (!pImpl_->rootWidget) return;
    
    // ===================================================================
    // MOUSE EVENTS: Use hit-testing to find target widget
    // ===================================================================
    if (auto* mouseMove = std::get_if<event::MouseMoveEvent>(&event)) {
        auto* target = pImpl_->rootWidget->hitTest(mouseMove->position);
        if (target) {
            // Try target first
            if (target->onEvent(event)) return;
            
            // Bubble up if not handled
            auto* parent = target->getParent();
            while (parent) {
                if (parent->onEvent(event)) return;
                parent = parent->getParent();
            }
        }
        return;
    }
    
    if (auto* mouseButton = std::get_if<event::MouseButtonEvent>(&event)) {
        auto* target = pImpl_->rootWidget->hitTest(mouseButton->position);
        if (target) {
            if (target->onEvent(event)) return;
            
            auto* parent = target->getParent();
            while (parent) {
                if (parent->onEvent(event)) return;
                parent = parent->getParent();
            }
        }
        return;
    }
    
    if (auto* mouseWheel = std::get_if<event::MouseWheelEvent>(&event)) {
        auto* target = pImpl_->rootWidget->hitTest(mouseWheel->position);
        if (target) {
            if (target->onEvent(event)) return;
            
            auto* parent = target->getParent();
            while (parent) {
                if (parent->onEvent(event)) return;
                parent = parent->getParent();
            }
        }
        return;
    }
    
    // ===================================================================
    // FILE DROP: Use hit-testing to find target widget
    // ===================================================================
    if (auto* fileDrop = std::get_if<event::FileDropEvent>(&event)) {
        auto* target = pImpl_->rootWidget->hitTest(fileDrop->position);
        if (target) {
            if (target->onEvent(event)) return;
            
            auto* parent = target->getParent();
            while (parent) {
                if (parent->onEvent(event)) return;
                parent = parent->getParent();
            }
        }
        return;
    }
    
    // ===================================================================
    // KEYBOARD EVENTS: Send to focused widget or root
    // ===================================================================
    if (std::holds_alternative<event::KeyEvent>(event)) {
        // TODO: Implement focus management
        // For now, send to root widget
        pImpl_->rootWidget->onEvent(event);
        return;
    }
    
    // ===================================================================
    // WINDOW EVENTS: Broadcast to entire tree
    // ===================================================================
    if (std::holds_alternative<event::ResizeEvent>(event) ||
        std::holds_alternative<event::PaintEvent>(event)) {
        pImpl_->rootWidget->onEvent(event);
        return;
    }
    
    // Default: broadcast to root
    pImpl_->rootWidget->onEvent(event);
}

} // namespace frqs::core