#include "window_impl.hpp"
#include "../../include/core/window_registry.hpp"

namespace frqs::platform {
    HWND createNativeWindow(const core::WindowParams& params, void* windowPtr);
}

namespace frqs::core {

// ============================================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================================

Window::Window(const WindowParams& params)
    : pImpl_(std::make_unique<Impl>())
    , id_{0} // Will be set by WindowRegistry
{
    pImpl_->title = params.title;
    pImpl_->size = params.size;
    pImpl_->position = params.position;
    pImpl_->resizable = params.resizable;
    pImpl_->visible = params.visible;
    pImpl_->decorated = params.decorated;

    // Initialize dirty rect manager
    pImpl_->initializeDirtyRects();

    try {
        pImpl_->hwnd = platform::createNativeWindow(params, this);
    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("Failed to create window: ") + e.what()
        );
    }
}

Window::~Window() noexcept {
    // TODO: Destroy native window
    // Cleanup handled by unique_ptr
}

// ============================================================================
// FACTORY METHOD
// ============================================================================

std::shared_ptr<Window> Window::create(const WindowParams& params) {
    auto window = std::shared_ptr<Window>(new Window(params));
    
    // Register with WindowRegistry
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
    if (pImpl_->size == size) return;
    
    pImpl_->size = size;
    pImpl_->updateDirtyRectBounds();
    
    if (pImpl_->hwnd) {
        // Get current window style
        DWORD style = static_cast<DWORD>(GetWindowLongPtrW(pImpl_->hwnd, GWL_STYLE));
        DWORD exStyle = static_cast<DWORD>(GetWindowLongPtrW(pImpl_->hwnd, GWL_EXSTYLE));
        
        // Calculate window size including borders
        RECT rect = {0, 0, static_cast<LONG>(size.w), static_cast<LONG>(size.h)};
        AdjustWindowRectEx(&rect, style, FALSE, exStyle);
        
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        
        SetWindowPos(pImpl_->hwnd, nullptr, 0, 0, width, height,
                     SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

widget::Size<uint32_t> Window::getSize() const noexcept {
    return pImpl_->size;
}

void Window::setPosition(const widget::Point<int32_t>& pos) {
    if (pImpl_->position == pos) return;
    
    pImpl_->position = pos;
    
    if (pImpl_->hwnd) {
        SetWindowPos(pImpl_->hwnd, nullptr, pos.x, pos.y, 0, 0,
                     SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
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
    
    // Unregister from WindowRegistry
    WindowRegistry::instance().unregisterWindow(id_);
}

// ============================================================================
// WIDGET TREE
// ============================================================================

void Window::setRootWidget(std::shared_ptr<widget::IWidget> root) {
    pImpl_->rootWidget = std::move(root);
    
    // Set root widget rect to match client area
    if (pImpl_->rootWidget) {
        pImpl_->rootWidget->setRect(getClientRect());
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
    
    // TODO: Request redraw from application
    // Application::instance().requestRender(id_);
}

void Window::invalidateRect(const widget::Rect<int32_t, uint32_t>& rect) noexcept {
    if (pImpl_->dirtyRects) {
        pImpl_->dirtyRects->addDirtyRect(rect);
    }
    
    // TODO: Request redraw from application
    // Application::instance().requestRender(id_);
}

void Window::forceRedraw() noexcept {
    invalidate();
    
    // TODO: Force immediate redraw
    // This would call the renderer directly
}

// ============================================================================
// UNSAFE BACKDOOR
// ============================================================================

void* Window::getNativeHandleUnsafe() const noexcept {
    return pImpl_->hwnd;
}

} // namespace frqs::core