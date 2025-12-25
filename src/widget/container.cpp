/**
 * @file container.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Implements the Container widget, a fundamental building block for creating complex UI layouts.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "widget/container.hpp"

namespace frqs::widget {

// ============================================================================
// CONTAINER IMPLEMENTATION
// ============================================================================

/**
 * @brief Constructs a new Container object.
 * 
 * By default, a container is initialized with an AbsoluteLayout, which allows
 * for manual positioning of child widgets.
 */
Container::Container()
    : Widget()
{
    // Default to absolute layout (manual positioning)
    layout_ = std::make_unique<AbsoluteLayout>();
}

/**
 * @brief Sets the layout manager for the container.
 * 
 * The layout manager is responsible for arranging the child widgets within the
 * container. If auto-layout is enabled, the new layout is applied immediately.
 * 
 * @param layout A unique pointer to an object implementing the ILayout interface.
 */
void Container::setLayout(std::unique_ptr<ILayout> layout) {
    layout_ = std::move(layout);
    if (autoLayout_) {
        applyLayout();
    }
}

/**
 * @brief Sets the padding for the container.
 * 
 * Padding is the space between the container's border and its content.
 * If auto-layout is enabled, the layout is reapplied to account for the new padding.
 * 
 * @param padding The padding value to be applied to all sides.
 */
void Container::setPadding(uint32_t padding) noexcept {
    if (padding_ == padding) return;
    padding_ = padding;
    if (autoLayout_) {
        applyLayout();
    }
}

/**
 * @brief Sets the border properties for the container.
 * 
 * @param color The color of the border.
 * @param width The width of the border in pixels.
 */
void Container::setBorder(const Color& color, float width) noexcept {
    borderColor_ = color;
    borderWidth_ = width;
    invalidate();
}

/**
 * @brief Applies the current layout to arrange the child widgets.
 * 
 * This function is called automatically when needed if auto-layout is enabled,
 * but can also be called manually to force a re-layout.
 */
void Container::applyLayout() {
    if (!layout_) return;
    
    layout_->apply(this);
    invalidate();
}

/**
 * @brief Sets the rectangle (position and size) of the container.
 * 
 * Overrides the base Widget::setRect to also re-apply the layout if auto-layout
 * is enabled, ensuring children are rearranged correctly when the container is resized.
 * 
 * @param rect The new rectangle for the container.
 */
void Container::setRect(const Rect<int32_t, uint32_t>& rect) {
    Widget::setRect(rect);
    
    // Auto-apply layout when container is resized
    if (autoLayout_ && layout_) {
        applyLayout();
    }
}

/**
 * @brief Renders the container and its child widgets.
 * 
 * This involves drawing the background, the border (if any), and then
 * recursively rendering all child widgets.
 * 
 * @param renderer The Renderer object to use for drawing operations.
 */
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