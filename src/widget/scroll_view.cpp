/**
 * @file scroll_view.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Implements the ScrollView widget, a container that allows scrolling of its content.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "widget/scroll_view.hpp"
#include "render/renderer.hpp"
#include <algorithm>

namespace frqs::widget {

// ============================================================================
// CONSTRUCTOR & CONTENT
// ============================================================================

/**
 * @brief Constructs a new ScrollView widget.
 */
ScrollView::ScrollView() : Widget() {
    setBackgroundColor(colors::White);
}

/**
 * @brief Sets the content widget to be displayed within the scroll view.
 * 
 * If there is existing content, it will be removed and replaced with the new content.
 * 
 * @param content A shared pointer to the widget to be used as content.
 */
void ScrollView::setContent(std::shared_ptr<IWidget> content) {
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

/**
 * @brief Scrolls the view to a specific offset.
 * 
 * The values will be clamped to the valid scrollable range.
 * 
 * @param x The horizontal scroll offset.
 * @param y The vertical scroll offset.
 */
void ScrollView::scrollTo(float x, float y) {
    scrollOffset_.x = x;
    scrollOffset_.y = y;
    clampScrollOffset();
    invalidate();
}

/**
 * @brief Scrolls the view by a given delta amount.
 * @param dx The change in horizontal scroll offset.
 * @param dy The change in vertical scroll offset.
 */
void ScrollView::scrollBy(float dx, float dy) {
    scrollOffset_.x += dx;
    scrollOffset_.y += dy;
    clampScrollOffset();
    invalidate();
}

/**
 * @brief Scrolls the view to the very bottom of the content.
 */
void ScrollView::scrollToBottom() {
    auto viewport = getViewportRect();
    float maxScrollY = static_cast<float>(contentSize_.h) - viewport.h;
    scrollTo(scrollOffset_.x, std::max(0.0f, maxScrollY));
}

// ============================================================================
// HIT-TEST OVERRIDE (CRITICAL FIX)
// ============================================================================

/**
 * @brief Performs a hit test to determine which widget is at a given point.
 * 
 * This override is crucial for ensuring that scrollbars and content are correctly
 * targeted by mouse events. It checks scrollbars first, then translates the point
 * into the content's coordinate space for further testing.
 * 
 * @param point The point to test, in the parent's coordinate system.
 * @return A pointer to the widget at the specified point, or `nullptr` if no widget is hit.
 */
IWidget* ScrollView::hitTest(const Point<int32_t>& point) {
    // 1. Check visibility
    if (!isVisible()) return nullptr;
    
    // 2. Check if point is inside viewport
    auto viewport = getViewportRect();
    if (point.x < viewport.x || point.x >= static_cast<int32_t>(viewport.getRight()) ||
        point.y < viewport.y || point.y >= static_cast<int32_t>(viewport.getBottom())) {
        return nullptr;
    }
    
    // 3. Check scrollbars first (they're on top)
    if (showScrollbars_) {
        auto vScrollbar = getVerticalScrollbarRect();
        if (vScrollbar.w > 0 &&
            point.x >= vScrollbar.x && point.x < static_cast<int32_t>(vScrollbar.getRight()) &&
            point.y >= vScrollbar.y && point.y < static_cast<int32_t>(vScrollbar.getBottom())) {
            return this; // Scrollbar was hit
        }
        
        auto hScrollbar = getHorizontalScrollbarRect();
        if (hScrollbar.h > 0 &&
            point.x >= hScrollbar.x && point.x < static_cast<int32_t>(hScrollbar.getRight()) &&
            point.y >= hScrollbar.y && point.y < static_cast<int32_t>(hScrollbar.getBottom())) {
            return this; // Scrollbar was hit
        }
    }
    
    // 4. Transform point to content space
    Point<int32_t> contentPoint = translateToContentSpace(point);
    
    // 5. Test content widget
    if (content_) {
        auto* result = content_->hitTest(contentPoint);
        if (result) return result;
    }
    
    // 6. ScrollView itself was hit
    return this;
}

// ============================================================================
// EVENT HANDLING
// ============================================================================

/**
 * @brief Sets the rectangle defining the widget's bounds.
 * @param rect The new widget bounds.
 */
void ScrollView::setRect(const Rect<int32_t, uint32_t>& rect) {
    Widget::setRect(rect);
    updateContentSize();
    clampScrollOffset();
}

/**
 * @brief Handles incoming events for the widget.
 * @param event The event to process.
 * @return `true` if the event was handled by the scroll view itself (e.g., scrollbar interaction), `false` otherwise.
 */
bool ScrollView::onEvent(const event::Event& event) {
    // Mouse wheel
    if (auto* wheelEvt = std::get_if<event::MouseWheelEvent>(&event)) {
        if (handleMouseWheel(*wheelEvt)) {
            return true;
        }
    }

    // Mouse button (for dragging scrollbars)
    if (auto* btnEvt = std::get_if<event::MouseButtonEvent>(&event)) {
        if (handleMouseButton(*btnEvt)) {
            return true;
        }
    }

    // Mouse move (for dragging + hover)
    if (auto* moveEvt = std::get_if<event::MouseMoveEvent>(&event)) {
        if (handleMouseMove(*moveEvt)) {
            return true;
        }
    }

    return false;
}

// ============================================================================
// MOUSE EVENT HANDLERS
// ============================================================================

/**
 * @brief Handles mouse wheel events to scroll the content.
 * @param evt The mouse wheel event.
 * @return `true` if the event was handled, `false` otherwise.
 * @internal
 */
bool ScrollView::handleMouseWheel(const event::MouseWheelEvent& evt) {
    auto viewport = getViewportRect();
    
    bool inside = 
        evt.position.x >= viewport.x && 
        evt.position.x < static_cast<int32_t>(viewport.getRight()) &&
        evt.position.y >= viewport.y && 
        evt.position.y < static_cast<int32_t>(viewport.getBottom());
    
    if (!inside) return false;
    
    // Scroll vertically
    scrollBy(0.0f, -static_cast<float>(evt.delta) / 4.0f);
    
    return true;
}

/**
 * @brief Handles mouse button events for scrollbar interaction (dragging and track clicking).
 * @param evt The mouse button event.
 * @return `true` if the event was handled, `false` otherwise.
 * @internal
 */
bool ScrollView::handleMouseButton(const event::MouseButtonEvent& evt) {
    if (evt.button != event::MouseButtonEvent::Button::Left) return false;
    
    if (evt.action == event::MouseButtonEvent::Action::Press) {
        // Check vertical scrollbar thumb
        auto vThumb = getVerticalThumbRect();
        if (vThumb.w > 0 &&
            evt.position.x >= vThumb.x && evt.position.x < static_cast<int32_t>(vThumb.getRight()) &&
            evt.position.y >= vThumb.y && evt.position.y < static_cast<int32_t>(vThumb.getBottom())) {
            draggingVScroll_ = true;
            dragStartPos_ = evt.position;
            dragStartOffset_ = scrollOffset_.y;
            return true;
        }
        
        // Check horizontal scrollbar thumb
        auto hThumb = getHorizontalThumbRect();
        if (hThumb.h > 0 &&
            evt.position.x >= hThumb.x && evt.position.x < static_cast<int32_t>(hThumb.getRight()) &&
            evt.position.y >= hThumb.y && evt.position.y < static_cast<int32_t>(hThumb.getBottom())) {
            draggingHScroll_ = true;
            dragStartPos_ = evt.position;
            dragStartOffset_ = scrollOffset_.x;
            return true;
        }
        
        // Check vertical scrollbar track (jump scroll)
        auto vScrollbar = getVerticalScrollbarRect();
        if (vScrollbar.w > 0 &&
            evt.position.x >= vScrollbar.x && evt.position.x < static_cast<int32_t>(vScrollbar.getRight()) &&
            evt.position.y >= vScrollbar.y && evt.position.y < static_cast<int32_t>(vScrollbar.getBottom())) {
            float ratio = static_cast<float>(evt.position.y - vScrollbar.y) / vScrollbar.h;
            auto viewport = getViewportRect();
            float maxScroll = std::max(0.0f, static_cast<float>(contentSize_.h - viewport.h));
            scrollTo(scrollOffset_.x, ratio * maxScroll);
            return true;
        }
        
        // Check horizontal scrollbar track
        auto hScrollbar = getHorizontalScrollbarRect();
        if (hScrollbar.h > 0 &&
            evt.position.x >= hScrollbar.x && evt.position.x < static_cast<int32_t>(hScrollbar.getRight()) &&
            evt.position.y >= hScrollbar.y && evt.position.y < static_cast<int32_t>(hScrollbar.getBottom())) {
            float ratio = static_cast<float>(evt.position.x - hScrollbar.x) / hScrollbar.w;
            auto viewport = getViewportRect();
            float maxScroll = std::max(0.0f, static_cast<float>(contentSize_.w - viewport.w));
            scrollTo(ratio * maxScroll, scrollOffset_.y);
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

/**
 * @brief Handles mouse move events for dragging scrollbar thumbs and updating hover states.
 * @param evt The mouse move event.
 * @return `true` if the event was handled, `false` otherwise.
 * @internal
 */
bool ScrollView::handleMouseMove(const event::MouseMoveEvent& evt) {
    // Handle dragging
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
// RENDER (CRITICAL FIX: Proper Transform Stack)
// ============================================================================

/**
 * @brief Renders the scroll view, its content, and scrollbars.
 * 
 * This method uses a clip rectangle to ensure content is only drawn within the
 * viewport. It then applies a translation transform to the renderer before
 * drawing the content, effectively scrolling it. Scrollbars are drawn on top
 * afterwards without the transform.
 * 
 * @param renderer The renderer to use for drawing.
 */
void ScrollView::render(Renderer& renderer) {
    if (!isVisible()) return;

    auto rect = getRect();
    auto viewport = getViewportRect();

    // Background
    renderer.fillRect(rect, getBackgroundColor());

    if (!content_) {
        renderScrollbars(renderer);
        return;
    }

    // ✅ CRITICAL FIX: Proper transform stack management
    auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer);
    if (!extRenderer) {
        // Fallback: render without transform
        content_->render(renderer);
        renderScrollbars(renderer);
        return;
    }

    // 1. Push clip to viewport
    renderer.pushClip(viewport);
    
    // 2. Save current transform
    extRenderer->save();
    
    // 3. Apply scroll translation
    extRenderer->translate(-scrollOffset_.x, -scrollOffset_.y);
    
    // 4. Render content (transformed)
    content_->render(renderer);
    
    // 5. Restore transform
    extRenderer->restore();
    
    // 6. Pop clip
    renderer.popClip();

    // 7. Render scrollbars on top (no transform)
    renderScrollbars(renderer);
}

// ============================================================================
// HELPER METHODS
// ============================================================================

/**
 * @brief Updates the cached size of the content widget.
 * @internal
 */
void ScrollView::updateContentSize() {
    if (!content_) {
        contentSize_ = Size<uint32_t>(0, 0);
        return;
    }

    auto contentRect = content_->getRect();
    contentSize_ = Size<uint32_t>(contentRect.w, contentRect.h);
}

/**
 * @brief Clamps the current scroll offset to be within the valid scrollable range.
 * @internal
 */
void ScrollView::clampScrollOffset() {
    auto viewport = getViewportRect();
    
    float maxScrollX = std::max(0.0f, static_cast<float>(contentSize_.w) - viewport.w);
    float maxScrollY = std::max(0.0f, static_cast<float>(contentSize_.h) - viewport.h);
    
    scrollOffset_.x = std::clamp(scrollOffset_.x, 0.0f, maxScrollX);
    scrollOffset_.y = std::clamp(scrollOffset_.y, 0.0f, maxScrollY);
}

/**
 * @brief Gets the rectangle for the viewport where content is displayed.
 * This excludes the area taken by visible scrollbars.
 * @return The viewport rectangle.
 * @internal
 */
Rect<int32_t, uint32_t> ScrollView::getViewportRect() const {
    auto rect = getRect();
    uint32_t w = rect.w;
    uint32_t h = rect.h;
    
    if (showScrollbars_ && verticalScrollEnabled_ && contentSize_.h > rect.h) {
        w = w > static_cast<uint32_t>(scrollbarWidth_) ? w - static_cast<uint32_t>(scrollbarWidth_) : 0;
    }
    
    if (showScrollbars_ && horizontalScrollEnabled_ && contentSize_.w > rect.w) {
        h = h > static_cast<uint32_t>(scrollbarWidth_) ? h - static_cast<uint32_t>(scrollbarWidth_) : 0;
    }
    
    return Rect<int32_t, uint32_t>(rect.x, rect.y, w, h);
}

/**
 * @brief Gets the rectangle for the vertical scrollbar track.
 * @return The scrollbar rectangle. Returns a zero-sized rect if not shown.
 * @internal
 */
Rect<int32_t, uint32_t> ScrollView::getVerticalScrollbarRect() const {
    auto rect = getRect();
    auto viewport = getViewportRect();
    
    if (!showScrollbars_ || !verticalScrollEnabled_ || contentSize_.h <= viewport.h) {
        return Rect<int32_t, uint32_t>(0, 0, 0, 0);
    }
    
    int32_t x = rect.x + static_cast<int32_t>(viewport.w);
    return Rect<int32_t, uint32_t>(x, rect.y, static_cast<uint32_t>(scrollbarWidth_), viewport.h);
}

/**
 * @brief Gets the rectangle for the horizontal scrollbar track.
 * @return The scrollbar rectangle. Returns a zero-sized rect if not shown.
 * @internal
 */
Rect<int32_t, uint32_t> ScrollView::getHorizontalScrollbarRect() const {
    auto rect = getRect();
    auto viewport = getViewportRect();
    
    if (!showScrollbars_ || !horizontalScrollEnabled_ || contentSize_.w <= viewport.w) {
        return Rect<int32_t, uint32_t>(0, 0, 0, 0);
    }
    
    int32_t y = rect.y + static_cast<int32_t>(viewport.h);
    return Rect<int32_t, uint32_t>(rect.x, y, viewport.w, static_cast<uint32_t>(scrollbarWidth_));
}

/**
 * @brief Gets the rectangle for the vertical scrollbar thumb (the draggable part).
 * @return The vertical thumb rectangle.
 * @internal
 */
Rect<int32_t, uint32_t> ScrollView::getVerticalThumbRect() const {
    auto scrollbarRect = getVerticalScrollbarRect();
    if (scrollbarRect.w == 0) return scrollbarRect;
    
    auto viewport = getViewportRect();
    float ratio = static_cast<float>(viewport.h) / contentSize_.h;
    uint32_t thumbHeight = std::max(static_cast<uint32_t>(scrollbarRect.h * ratio), 20u);
    
    float maxScroll = std::max(0.0f, static_cast<float>(contentSize_.h - viewport.h));
    float scrollRatio = maxScroll > 0.0f ? (scrollOffset_.y / maxScroll) : 0.0f;
    
    int32_t thumbY = scrollbarRect.y + static_cast<int32_t>((scrollbarRect.h - thumbHeight) * scrollRatio);
    
    return Rect<int32_t, uint32_t>(scrollbarRect.x, thumbY, scrollbarRect.w, thumbHeight);
}

/**
 * @brief Gets the rectangle for the horizontal scrollbar thumb (the draggable part).
 * @return The horizontal thumb rectangle.
 * @internal
 */
Rect<int32_t, uint32_t> ScrollView::getHorizontalThumbRect() const {
    auto scrollbarRect = getHorizontalScrollbarRect();
    if (scrollbarRect.h == 0) return scrollbarRect;
    
    auto viewport = getViewportRect();
    float ratio = static_cast<float>(viewport.w) / contentSize_.w;
    uint32_t thumbWidth = std::max(static_cast<uint32_t>(scrollbarRect.w * ratio), 20u);
    
    float maxScroll = std::max(0.0f, static_cast<float>(contentSize_.w - viewport.w));
    float scrollRatio = maxScroll > 0.0f ? (scrollOffset_.x / maxScroll) : 0.0f;
    
    int32_t thumbX = scrollbarRect.x + static_cast<int32_t>((scrollbarRect.w - thumbWidth) * scrollRatio);
    
    return Rect<int32_t, uint32_t>(thumbX, scrollbarRect.y, thumbWidth, scrollbarRect.h);
}

/**
 * @brief Renders the vertical and horizontal scrollbars, if needed.
 * @param renderer The renderer to use for drawing.
 * @internal
 */
void ScrollView::renderScrollbars(Renderer& renderer) {
    if (!showScrollbars_) return;
    
    auto viewport = getViewportRect();
    
    // Vertical scrollbar
    if (verticalScrollEnabled_ && contentSize_.h > viewport.h) {
        auto scrollbarRect = getVerticalScrollbarRect();
        auto thumbRect = getVerticalThumbRect();
        
        renderer.fillRect(scrollbarRect, Color(240, 240, 240, 200));
        
        Color thumbColor = hoveringVScroll_ ? scrollbarHoverColor_ : scrollbarColor_;
        renderer.fillRect(thumbRect, thumbColor);
    }
    
    // Horizontal scrollbar
    if (horizontalScrollEnabled_ && contentSize_.w > viewport.w) {
        auto scrollbarRect = getHorizontalScrollbarRect();
        auto thumbRect = getHorizontalThumbRect();
        
        renderer.fillRect(scrollbarRect, Color(240, 240, 240, 200));
        
        Color thumbColor = hoveringHScroll_ ? scrollbarHoverColor_ : scrollbarColor_;
        renderer.fillRect(thumbRect, thumbColor);
    }
}

/**
 * @brief Translates a point from the scroll view's coordinate system to the content's coordinate system.
 * @param screenPoint The point in the scroll view's coordinates.
 * @return The corresponding point in the content's coordinates.
 * @internal
 */
Point<int32_t> ScrollView::translateToContentSpace(const Point<int32_t>& screenPoint) const {
    auto viewport = getViewportRect();
    return Point<int32_t>(
        screenPoint.x - viewport.x + static_cast<int32_t>(scrollOffset_.x),
        screenPoint.y - viewport.y + static_cast<int32_t>(scrollOffset_.y)
    );
}

/**
 * @brief Checks if a point is within the vertical scrollbar thumb.
 * @param point The point to check.
 * @return `true` if the point is inside the thumb, `false` otherwise.
 * @internal
 */
bool ScrollView::isPointInVerticalScrollbar(const Point<int32_t>& point) const {
    auto thumbRect = getVerticalThumbRect();
    return thumbRect.w > 0 && 
           point.x >= thumbRect.x && point.x < static_cast<int32_t>(thumbRect.getRight()) &&
           point.y >= thumbRect.y && point.y < static_cast<int32_t>(thumbRect.getBottom());
}

/**
 * @brief Checks if a point is within the horizontal scrollbar thumb.
 * @param point The point to check.
 * @return `true` if the point is inside the thumb, `false` otherwise.
 * @internal
 */
bool ScrollView::isPointInHorizontalScrollbar(const Point<int32_t>& point) const {
    auto thumbRect = getHorizontalThumbRect();
    return thumbRect.h > 0 && 
           point.x >= thumbRect.x && point.x < static_cast<int32_t>(thumbRect.getRight()) &&
           point.y >= thumbRect.y && point.y < static_cast<int32_t>(thumbRect.getBottom());
}

} // namespace frqs::widget
