#include "../include/widget/iwidget.hpp"
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

    Impl() = default;
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

    // Remove from previous parent if any
    if (auto* childWidget = dynamic_cast<Widget*>(child.get())) {
        if (childWidget->pImpl_->parent) {
            childWidget->pImpl_->parent->removeChild(child.get());
        }
        childWidget->pImpl_->parent = this;
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
// INVALIDATION (Placeholder - needs Window integration)
// ============================================================================

void Widget::invalidate() noexcept {
    // TODO: Notify parent window to mark this rect as dirty
    // For now, just a placeholder
}

void Widget::invalidateRect(const Rect<int32_t, uint32_t>& rect) noexcept {
    // TODO: Notify parent window to mark specific rect as dirty
    (void)rect;
}

} // namespace frqs::widget