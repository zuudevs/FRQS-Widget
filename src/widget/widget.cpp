// src/widget/widget.cpp - COMPLETE FILE dengan EVENT HANDLING FIX
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
    
    // Layout properties
    LayoutProps layoutProps;
    
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
// LAYOUT PROPERTIES
// ============================================================================

void Widget::setLayoutWeight(float weight) noexcept {
    if (pImpl_->layoutProps.weight == weight) return;
    pImpl_->layoutProps.weight = weight;
    
    if (pImpl_->parent) {
        if (auto* parentWidget = dynamic_cast<Widget*>(pImpl_->parent)) {
            parentWidget->invalidate();
        }
    }
}

float Widget::getLayoutWeight() const noexcept {
    return pImpl_->layoutProps.weight;
}

void Widget::setMinSize(int32_t width, int32_t height) noexcept {
    pImpl_->layoutProps.minWidth = width;
    pImpl_->layoutProps.minHeight = height;
    if (pImpl_->parent) {
        if (auto* parentWidget = dynamic_cast<Widget*>(pImpl_->parent)) {
            parentWidget->invalidate();
        }
    }
}

void Widget::setMaxSize(int32_t width, int32_t height) noexcept {
    pImpl_->layoutProps.maxWidth = width;
    pImpl_->layoutProps.maxHeight = height;
    if (pImpl_->parent) {
        if (auto* parentWidget = dynamic_cast<Widget*>(pImpl_->parent)) {
            parentWidget->invalidate();
        }
    }
}

void Widget::setMinWidth(int32_t width) noexcept {
    pImpl_->layoutProps.minWidth = width;
    if (pImpl_->parent) {
        if (auto* parentWidget = dynamic_cast<Widget*>(pImpl_->parent)) {
            parentWidget->invalidate();
        }
    }
}

void Widget::setMaxWidth(int32_t width) noexcept {
    pImpl_->layoutProps.maxWidth = width;
    if (pImpl_->parent) {
        if (auto* parentWidget = dynamic_cast<Widget*>(pImpl_->parent)) {
            parentWidget->invalidate();
        }
    }
}

void Widget::setMinHeight(int32_t height) noexcept {
    pImpl_->layoutProps.minHeight = height;
    if (pImpl_->parent) {
        if (auto* parentWidget = dynamic_cast<Widget*>(pImpl_->parent)) {
            parentWidget->invalidate();
        }
    }
}

void Widget::setMaxHeight(int32_t height) noexcept {
    pImpl_->layoutProps.maxHeight = height;
    if (pImpl_->parent) {
        if (auto* parentWidget = dynamic_cast<Widget*>(pImpl_->parent)) {
            parentWidget->invalidate();
        }
    }
}

void Widget::setAlignSelf(LayoutProps::Align align) noexcept {
    if (pImpl_->layoutProps.alignSelf == align) return;
    pImpl_->layoutProps.alignSelf = align;
    if (pImpl_->parent) {
        if (auto* parentWidget = dynamic_cast<Widget*>(pImpl_->parent)) {
            parentWidget->invalidate();
        }
    }
}

LayoutProps::Align Widget::getAlignSelf() const noexcept {
    return pImpl_->layoutProps.alignSelf;
}

const LayoutProps& Widget::getLayoutProps() const noexcept {
    return pImpl_->layoutProps;
}

LayoutProps& Widget::getLayoutPropsMut() noexcept {
    return pImpl_->layoutProps;
}

// ============================================================================
// EVENT HANDLING - FIXED VERSION
// ============================================================================

bool Widget::onEvent(const event::Event& event) {
    // ✅ CRITICAL FIX: For mouse move events, dispatch to ALL children
    // This ensures hover states are updated correctly
    if (std::holds_alternative<event::MouseMoveEvent>(event)) {
        bool handled = false;
        
        // Send to ALL children (don't stop at first handler)
        for (auto& child : pImpl_->children) {
            if (child->onEvent(event)) {
                handled = true;
                // ✅ Continue to other children!
            }
        }
        
        return handled;
    }
    
    // ✅ For other events (click, key, wheel, etc), stop at first handler
    for (auto& child : pImpl_->children) {
        if (child->onEvent(event)) {
            return true;  // Event handled, stop propagation
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
        childWidget->pImpl_->windowHandle = pImpl_->getWindowHandle();
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
            childWidget->pImpl_->windowHandle = nullptr;
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
// INVALIDATION
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