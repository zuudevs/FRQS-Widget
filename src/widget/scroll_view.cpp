#include "widget/scroll_view.hpp"
#include "render/renderer.hpp"
#include <algorithm>
#include <cmath>

namespace frqs::widget {

// ============================================================================
// CONSTRUCTOR
// ============================================================================

ScrollView::ScrollView()
    : Widget()
{
    setBackgroundColor(colors::White);
}

// ============================================================================
// CONTENT MANAGEMENT
// ============================================================================

void ScrollView::setContent(std::shared_ptr<IWidget> content) {
    // Remove old content
    if (content_) {
        removeChild(content_.get());
    }

    content_ = content;

    if (content_) {
        addChild(content_);
        updateContentSize();
        clampScrollOffset();
    }

    invalidate();
}

// ============================================================================
// SCROLLING CONTROL
// ============================================================================

void ScrollView::scrollTo(float x, float y) {
    scrollOffset_.x = x;
    scrollOffset_.y = y;
    clampScrollOffset();
    invalidate();
}

void ScrollView::scrollBy(float dx, float dy) {
    scrollOffset_.x += dx;
    scrollOffset_.y += dy;
    clampScrollOffset();
    invalidate();
}

void ScrollView::scrollToBottom() {
    auto viewport = getViewportRect();
    float maxScrollY = static_cast<float>(contentSize_.h) - viewport.h;
    scrollTo(scrollOffset_.x, std::max(0.0f, maxScrollY));
}

// ============================================================================
// WIDGET OVERRIDES
// ============================================================================

void ScrollView::setRect(const Rect<int32_t, uint32_t>& rect) {
    Widget::setRect(rect);
    updateContentSize();
    clampScrollOffset();
}

bool ScrollView::onEvent(const event::Event& event) {
    // Handle mouse wheel
    if (auto* wheelEvt = std::get_if<event::MouseWheelEvent>(&event)) {
        return handleMouseWheel(*wheelEvt);
    }

    // Handle mouse button (for scrollbar dragging)
    if (auto* btnEvt = std::get_if<event::MouseButtonEvent>(&event)) {
        if (handleMouseButton(*btnEvt)) {
            return true;
        }
    }

    // Handle mouse move (for scrollbar hover and dragging)
    if (auto* moveEvt = std::get_if<event::MouseMoveEvent>(&event)) {
        if (handleMouseMove(*moveEvt)) {
            return true;
        }
    }

    // Transform event and pass to content
    if (content_) {
        auto transformedEvent = translateEvent(event);
        if (content_->onEvent(transformedEvent)) {
            return true;
        }
    }

    return Widget::onEvent(event);
}

void ScrollView::render(Renderer& renderer) {
    if (!isVisible()) return;

    auto rect = getRect();
    auto viewport = getViewportRect();

    // Render background
    renderer.fillRect(rect, getBackgroundColor());

    if (!content_) {
        renderScrollbars(renderer);
        return;
    }

    // Try to use extended renderer for transform support
    auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer);
    if (!extRenderer) {
        // Fallback: render without scrolling
        content_->render(renderer);
        return;
    }

    // ========================================================================
    // RENDER CONTENT WITH CLIPPING AND TRANSLATION
    // ========================================================================

    // 1. Push clip rect (viewport)
    renderer.pushClip(viewport);

    // 2. Save transform state
    extRenderer->save();

    // 3. Apply translation (scroll offset)
    extRenderer->translate(-scrollOffset_.x, -scrollOffset_.y);

    // 4. Render content (clipped and translated)
    content_->render(renderer);

    // 5. Restore transform
    extRenderer->restore();

    // 6. Pop clip
    renderer.popClip();

    // ========================================================================
    // RENDER SCROLLBARS ON TOP (NO CLIPPING)
    // ========================================================================

    renderScrollbars(renderer);
}

// ============================================================================
// INTERNAL HELPERS - GEOMETRY
// ============================================================================

void ScrollView::updateContentSize() {
    if (!content_) {
        contentSize_ = Size<uint32_t>(0, 0);
        return;
    }

    auto contentRect = content_->getRect();
    contentSize_ = Size<uint32_t>(contentRect.w, contentRect.h);
}

void ScrollView::clampScrollOffset() {
    auto viewport = getViewportRect();

    float maxScrollX = std::max(0.0f, static_cast<float>(contentSize_.w) - viewport.w);
    float maxScrollY = std::max(0.0f, static_cast<float>(contentSize_.h) - viewport.h);

    scrollOffset_.x = std::clamp(scrollOffset_.x, 0.0f, maxScrollX);
    scrollOffset_.y = std::clamp(scrollOffset_.y, 0.0f, maxScrollY);
}

Rect<int32_t, uint32_t> ScrollView::getViewportRect() const {
    auto rect = getRect();

    // Reserve space for scrollbars if visible
    uint32_t w = rect.w;
    uint32_t h = rect.h;

    if (showScrollbars_ && verticalScrollEnabled_ && contentSize_.h > rect.h) {
        w = w > static_cast<uint32_t>(scrollbarWidth_) 
            ? w - static_cast<uint32_t>(scrollbarWidth_) 
            : 0;
    }

    if (showScrollbars_ && horizontalScrollEnabled_ && contentSize_.w > rect.w) {
        h = h > static_cast<uint32_t>(scrollbarWidth_) 
            ? h - static_cast<uint32_t>(scrollbarWidth_) 
            : 0;
    }

    return Rect<int32_t, uint32_t>(rect.x, rect.y, w, h);
}

Rect<int32_t, uint32_t> ScrollView::getVerticalScrollbarRect() const {
    auto rect = getRect();
    auto viewport = getViewportRect();

    if (!showScrollbars_ || !verticalScrollEnabled_ || contentSize_.h <= viewport.h) {
        return Rect<int32_t, uint32_t>(0, 0, 0, 0);
    }

    int32_t x = rect.x + static_cast<int32_t>(viewport.w);
    int32_t y = rect.y;
    uint32_t w = static_cast<uint32_t>(scrollbarWidth_);
    uint32_t h = viewport.h;

    return Rect<int32_t, uint32_t>(x, y, w, h);
}

Rect<int32_t, uint32_t> ScrollView::getHorizontalScrollbarRect() const {
    auto rect = getRect();
    auto viewport = getViewportRect();

    if (!showScrollbars_ || !horizontalScrollEnabled_ || contentSize_.w <= viewport.w) {
        return Rect<int32_t, uint32_t>(0, 0, 0, 0);
    }

    int32_t x = rect.x;
    int32_t y = rect.y + static_cast<int32_t>(viewport.h);
    uint32_t w = viewport.w;
    uint32_t h = static_cast<uint32_t>(scrollbarWidth_);

    return Rect<int32_t, uint32_t>(x, y, w, h);
}

Rect<int32_t, uint32_t> ScrollView::getVerticalThumbRect() const {
    auto scrollbarRect = getVerticalScrollbarRect();
    if (scrollbarRect.w == 0) return scrollbarRect;

    auto viewport = getViewportRect();

    // Calculate thumb size (proportional to viewport/content ratio)
    float ratio = static_cast<float>(viewport.h) / contentSize_.h;
    uint32_t thumbHeight = static_cast<uint32_t>(scrollbarRect.h * ratio);
    thumbHeight = std::max(thumbHeight, 20u); // Minimum thumb size

    // Calculate thumb position
    float maxScroll = std::max(0.0f, static_cast<float>(contentSize_.h - viewport.h));
    float scrollRatio = maxScroll > 0.0f ? (scrollOffset_.y / maxScroll) : 0.0f;
    uint32_t maxThumbOffset = scrollbarRect.h - thumbHeight;
    int32_t thumbY = scrollbarRect.y + static_cast<int32_t>(maxThumbOffset * scrollRatio);

    return Rect<int32_t, uint32_t>(
        scrollbarRect.x,
        thumbY,
        scrollbarRect.w,
        thumbHeight
    );
}

Rect<int32_t, uint32_t> ScrollView::getHorizontalThumbRect() const {
    auto scrollbarRect = getHorizontalScrollbarRect();
    if (scrollbarRect.h == 0) return scrollbarRect;

    auto viewport = getViewportRect();

    // Calculate thumb size
    float ratio = static_cast<float>(viewport.w) / contentSize_.w;
    uint32_t thumbWidth = static_cast<uint32_t>(scrollbarRect.w * ratio);
    thumbWidth = std::max(thumbWidth, 20u);

    // Calculate thumb position
    float maxScroll = std::max(0.0f, static_cast<float>(contentSize_.w - viewport.w));
    float scrollRatio = maxScroll > 0.0f ? (scrollOffset_.x / maxScroll) : 0.0f;
    uint32_t maxThumbOffset = scrollbarRect.w - thumbWidth;
    int32_t thumbX = scrollbarRect.x + static_cast<int32_t>(maxThumbOffset * scrollRatio);

    return Rect<int32_t, uint32_t>(
        thumbX,
        scrollbarRect.y,
        thumbWidth,
        scrollbarRect.h
    );
}

// ============================================================================
// RENDERING - SCROLLBARS
// ============================================================================

void ScrollView::renderScrollbars(Renderer& renderer) {
    if (!showScrollbars_) return;

    auto viewport = getViewportRect();

    // Vertical scrollbar
    if (verticalScrollEnabled_ && contentSize_.h > viewport.h) {
        auto scrollbarRect = getVerticalScrollbarRect();
        auto thumbRect = getVerticalThumbRect();

        // Track background
        renderer.fillRect(scrollbarRect, Color(240, 240, 240, 200));

        // Thumb
        Color thumbColor = hoveringVScroll_ ? scrollbarHoverColor_ : scrollbarColor_;
        renderer.fillRect(thumbRect, thumbColor);
    }

    // Horizontal scrollbar
    if (horizontalScrollEnabled_ && contentSize_.w > viewport.w) {
        auto scrollbarRect = getHorizontalScrollbarRect();
        auto thumbRect = getHorizontalThumbRect();

        // Track background
        renderer.fillRect(scrollbarRect, Color(240, 240, 240, 200));

        // Thumb
        Color thumbColor = hoveringHScroll_ ? scrollbarHoverColor_ : scrollbarColor_;
        renderer.fillRect(thumbRect, thumbColor);
    }
}

// ============================================================================
// EVENT HANDLING
// ============================================================================

bool ScrollView::handleMouseWheel(const event::MouseWheelEvent& evt) {
    auto viewport = getViewportRect();

    // Check if mouse is inside viewport
    bool inside = evt.position.x >= viewport.x && 
                  evt.position.x < viewport.getRight() &&
                  evt.position.y >= viewport.y && 
                  evt.position.y < viewport.getBottom();

    if (!inside) return false;

    // Scroll by wheel delta (negative = scroll down)
    float delta = static_cast<float>(evt.delta);
    float scrollAmount = -delta / 4.0f; // Adjust sensitivity

    // Vertical scrolling by default
    scrollBy(0.0f, scrollAmount);

    return true;
}

bool ScrollView::handleMouseButton(const event::MouseButtonEvent& evt) {
    if (evt.button != event::MouseButtonEvent::Button::Left) {
        return false;
    }

    if (evt.action == event::MouseButtonEvent::Action::Press) {
        // Check if clicking on scrollbar thumbs
        auto vThumb = getVerticalThumbRect();
        auto hThumb = getHorizontalThumbRect();

        bool inVThumb = evt.position.x >= vThumb.x && 
                        evt.position.x < vThumb.getRight() &&
                        evt.position.y >= vThumb.y && 
                        evt.position.y < vThumb.getBottom();

        bool inHThumb = evt.position.x >= hThumb.x && 
                        evt.position.x < hThumb.getRight() &&
                        evt.position.y >= hThumb.y && 
                        evt.position.y < hThumb.getBottom();

        if (inVThumb) {
            draggingVScroll_ = true;
            dragStartPos_ = evt.position;
            dragStartOffset_ = scrollOffset_.y;
            return true;
        }

        if (inHThumb) {
            draggingHScroll_ = true;
            dragStartPos_ = evt.position;
            dragStartOffset_ = scrollOffset_.x;
            return true;
        }

    } else if (evt.action == event::MouseButtonEvent::Action::Release) {
        if (draggingVScroll_ || draggingHScroll_) {
            draggingVScroll_ = false;
            draggingHScroll_ = false;
            return true;
        }
    }

    return false;
}

bool ScrollView::handleMouseMove(const event::MouseMoveEvent& evt) {
    // Handle scrollbar dragging
    if (draggingVScroll_) {
        auto viewport = getViewportRect();
        auto scrollbarRect = getVerticalScrollbarRect();
        auto thumbRect = getVerticalThumbRect();

        int32_t deltaY = evt.position.y - dragStartPos_.y;
        float maxScroll = std::max(0.0f, static_cast<float>(contentSize_.h - viewport.h));
        uint32_t maxThumbOffset = scrollbarRect.h - thumbRect.h;

        if (maxThumbOffset > 0) {
            float scrollDelta = (static_cast<float>(deltaY) / maxThumbOffset) * maxScroll;
            scrollTo(scrollOffset_.x, dragStartOffset_ + scrollDelta);
        }

        return true;
    }

    if (draggingHScroll_) {
        auto viewport = getViewportRect();
        auto scrollbarRect = getHorizontalScrollbarRect();
        auto thumbRect = getHorizontalThumbRect();

        int32_t deltaX = evt.position.x - dragStartPos_.x;
        float maxScroll = std::max(0.0f, static_cast<float>(contentSize_.w - viewport.w));
        uint32_t maxThumbOffset = scrollbarRect.w - thumbRect.w;

        if (maxThumbOffset > 0) {
            float scrollDelta = (static_cast<float>(deltaX) / maxThumbOffset) * maxScroll;
            scrollTo(dragStartOffset_ + scrollDelta, scrollOffset_.y);
        }

        return true;
    }

    // Update hover state
    bool wasHoveringV = hoveringVScroll_;
    bool wasHoveringH = hoveringHScroll_;

    hoveringVScroll_ = isPointInVerticalScrollbar(evt.position);
    hoveringHScroll_ = isPointInHorizontalScrollbar(evt.position);

    if (wasHoveringV != hoveringVScroll_ || wasHoveringH != hoveringHScroll_) {
        invalidate();
    }

    return false;
}

// ============================================================================
// COORDINATE TRANSFORMATION
// ============================================================================

Point<int32_t> ScrollView::translateToContentSpace(const Point<int32_t>& screenPoint) const {
    auto viewport = getViewportRect();

    return Point<int32_t>(
        screenPoint.x - viewport.x + static_cast<int32_t>(scrollOffset_.x),
        screenPoint.y - viewport.y + static_cast<int32_t>(scrollOffset_.y)
    );
}

event::Event ScrollView::translateEvent(const event::Event& event) const {
    // Transform mouse events to content space
    if (auto* mouseMove = std::get_if<event::MouseMoveEvent>(&event)) {
        event::MouseMoveEvent transformed = *mouseMove;
        transformed.position = translateToContentSpace(mouseMove->position);
        return transformed;
    }

    if (auto* mouseBtn = std::get_if<event::MouseButtonEvent>(&event)) {
        event::MouseButtonEvent transformed = *mouseBtn;
        transformed.position = translateToContentSpace(mouseBtn->position);
        return transformed;
    }

    if (auto* mouseWheel = std::get_if<event::MouseWheelEvent>(&event)) {
        event::MouseWheelEvent transformed = *mouseWheel;
        transformed.position = translateToContentSpace(mouseWheel->position);
        return transformed;
    }

    return event;
}

// ============================================================================
// HIT TESTING
// ============================================================================

bool ScrollView::isPointInVerticalScrollbar(const Point<int32_t>& point) const {
    auto thumbRect = getVerticalThumbRect();
    if (thumbRect.w == 0) return false;

    return point.x >= thumbRect.x && 
           point.x < thumbRect.getRight() &&
           point.y >= thumbRect.y && 
           point.y < thumbRect.getBottom();
}

bool ScrollView::isPointInHorizontalScrollbar(const Point<int32_t>& point) const {
    auto thumbRect = getHorizontalThumbRect();
    if (thumbRect.h == 0) return false;

    return point.x >= thumbRect.x && 
           point.x < thumbRect.getRight() &&
           point.y >= thumbRect.y && 
           point.y < thumbRect.getBottom();
}

} // namespace frqs::widget