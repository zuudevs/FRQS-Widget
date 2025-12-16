#include "widget/container.hpp"

namespace frqs::widget {

// ============================================================================
// CONTAINER IMPLEMENTATION
// ============================================================================

Container::Container()
    : Widget()
{
    // Default to absolute layout (manual positioning)
    layout_ = std::make_unique<AbsoluteLayout>();
}

void Container::setLayout(std::unique_ptr<ILayout> layout) {
    layout_ = std::move(layout);
    if (autoLayout_) {
        applyLayout();
    }
}

void Container::setPadding(uint32_t padding) noexcept {
    if (padding_ == padding) return;
    padding_ = padding;
    if (autoLayout_) {
        applyLayout();
    }
}

void Container::setBorder(const Color& color, float width) noexcept {
    borderColor_ = color;
    borderWidth_ = width;
    invalidate();
}

void Container::applyLayout() {
    if (!layout_) return;
    
    layout_->apply(this);
    invalidate();
}

void Container::setRect(const Rect<int32_t, uint32_t>& rect) {
    Widget::setRect(rect);
    
    // Auto-apply layout when container is resized
    if (autoLayout_ && layout_) {
        applyLayout();
    }
}

void Container::render(Renderer& renderer) {
    if (!isVisible()) return;

    auto rect = getRect();
    
    // Render background
    renderer.fillRect(rect, getBackgroundColor());
    
    // Render border if present
    if (borderWidth_ > 0.0f && borderColor_.a > 0) {
        renderer.drawRect(rect, borderColor_, borderWidth_);
    }
    
    // Render children (Widget::render will do this)
    Widget::render(renderer);
}

} // namespace frqs::widget