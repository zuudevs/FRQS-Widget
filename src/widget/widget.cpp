#include "widget/iwidget.hpp"
#include "core/window.hpp"
#include "platform/win32_safe.hpp"
#include <algorithm>

namespace frqs::widget {

// ============================================================================
// WIDGET PIMPL IMPLEMENTATION
// ============================================================================

struct Widget::Impl {
    Rect<int32_t, uint32_t> rect;
    Color backgroundColor = colors::White;
    bool visible = true;
    IWidget* parent = nullptr;
    std::vector<std::shared_ptr<IWidget>> children;
    HWND windowHandle = nullptr;
    Impl() = default;
    
    HWND getWindowHandle() {
        if (windowHandle) return windowHandle;
        if (parent) {
            if (auto* parentWidget = dynamic_cast<Widget*>(parent)) {
                return parentWidget->pImpl_->getWindowHandle();
            }
        }
        
        return nullptr;
    }
};

// ============================================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================================

Widget::Widget() : pImpl_(std::make_unique<Impl>()) {}

Widget::~Widget() noexcept = default;

Widget::Widget(Widget&& other) noexcept = default;

Widget& Widget::operator=(Widget&& other) noexcept = default;

// ============================================================================
// LAYOUT METHODS
// ============================================================================

void Widget::setRect(const Rect<int32_t, uint32_t>& rect) {
    if (pImpl_->rect == rect) return;
    
    pImpl_->rect = rect;
    invalidate();
}

Rect<int32_t, uint32_t> Widget::getRect() const noexcept {
    return pImpl_->rect;
}

void Widget::setVisible(bool visible) noexcept {
    if (pImpl_->visible == visible) return;
    
    pImpl_->visible = visible;
    invalidate();
}

bool Widget::isVisible() const noexcept {
    return pImpl_->visible;
}

// ============================================================================
// EVENT HANDLING
// ============================================================================

bool Widget::onEvent(const event::Event& event) {
    // Base implementation: propagate to children
    for (auto& child : pImpl_->children) {
        if (child->onEvent(event)) {
            return true;  // Event handled by child
        }
    }
    
    return false;  // Event not handled
}

// ============================================================================
// RENDERING
// ============================================================================

void Widget::render(Renderer& renderer) {
    if (!pImpl_->visible) return;

    // Render background
    renderer.fillRect(pImpl_->rect, pImpl_->backgroundColor);

    // Render children
    for (auto& child : pImpl_->children) {
        if (child->isVisible()) {
            child->render(renderer);
        }
    }
}

// ============================================================================
// HIERARCHY
// ============================================================================

void Widget::addChild(std::shared_ptr<IWidget> child) {
    if (!child) return;

    if (auto* childWidget = dynamic_cast<Widget*>(child.get())) {
        if (childWidget->pImpl_->parent) {
            childWidget->pImpl_->parent->removeChild(child.get());
        }
        childWidget->pImpl_->parent = this;
        childWidget->pImpl_->windowHandle = pImpl_->getWindowHandle();  // NEW!
    }

    pImpl_->children.push_back(std::move(child));
    invalidate();
}

void Widget::removeChild(IWidget* child) {
    if (!child) return;

    auto it = std::find_if(pImpl_->children.begin(), pImpl_->children.end(),
        [child](const auto& ptr) { return ptr.get() == child; });

    if (it != pImpl_->children.end()) {
        if (auto* childWidget = dynamic_cast<Widget*>(child)) {
            childWidget->pImpl_->parent = nullptr;
            childWidget->pImpl_->windowHandle = nullptr;  // NEW!
        }
        pImpl_->children.erase(it);
        invalidate();
    }
}

const std::vector<std::shared_ptr<IWidget>>& Widget::getChildren() const noexcept {
    return pImpl_->children;
}

IWidget* Widget::getParent() const noexcept {
    return pImpl_->parent;
}

// ============================================================================
// COLOR MANAGEMENT
// ============================================================================

void Widget::setBackgroundColor(const Color& color) noexcept {
    if (pImpl_->backgroundColor == color) return;
    
    pImpl_->backgroundColor = color;
    invalidate();
}

Color Widget::getBackgroundColor() const noexcept {
    return pImpl_->backgroundColor;
}

// ============================================================================
// INVALIDATION - ✅ FIXED: Actually trigger window redraw
// ============================================================================

void Widget::invalidate() noexcept {
    HWND hwnd = pImpl_->getWindowHandle();
    if (!hwnd) return;
    
    auto rect = pImpl_->rect;
    RECT r = {
        static_cast<LONG>(rect.x),
        static_cast<LONG>(rect.y),
        static_cast<LONG>(rect.getRight()),
        static_cast<LONG>(rect.getBottom())
    };
    
    InvalidateRect(hwnd, &r, FALSE);
}

void Widget::invalidateRect(const Rect<int32_t, uint32_t>& rect) noexcept {
    HWND hwnd = pImpl_->getWindowHandle();
    if (!hwnd) return;
    
    RECT r = {
        static_cast<LONG>(rect.x),
        static_cast<LONG>(rect.y),
        static_cast<LONG>(rect.getRight()),
        static_cast<LONG>(rect.getBottom())
    };
    
    InvalidateRect(hwnd, &r, FALSE);
}

namespace internal {
    void setWidgetWindowHandle(Widget* widget, void* hwnd) {
        if (!widget) return;
        widget->pImpl_->windowHandle = static_cast<HWND>(hwnd);
        
        for (auto& child : widget->pImpl_->children) {
            if (auto* childWidget = dynamic_cast<Widget*>(child.get())) {
                setWidgetWindowHandle(childWidget, hwnd);
            }
        }
    }
}

} // namespace frqs::widget