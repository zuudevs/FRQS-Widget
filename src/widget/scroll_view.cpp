// src/widget/scroll_view.cpp - COMPLETE with Draggable Scrollbars
#include "widget/scroll_view.hpp"
#include "render/renderer.hpp"
#include <algorithm>

namespace frqs::widget {

// ============================================================================
// CONSTRUCTOR & CONTENT
// ============================================================================

ScrollView::ScrollView() : Widget() {
    setBackgroundColor(colors::White);
}

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
// EVENT HANDLING (UPDATED with Drag Logic)
// ============================================================================

void ScrollView::setRect(const Rect<int32_t, uint32_t>& rect) {
    Widget::setRect(rect);
    updateContentSize();
    clampScrollOffset();
}

bool ScrollView::onEvent(const event::Event& event) {
    // Mouse wheel
    if (auto* wheelEvt = std::get_if<event::MouseWheelEvent>(&event)) {
        if (handleMouseWheel(*wheelEvt)) {
            return true;
        }
    }

    // Mouse button (NEW - for dragging)
    if (auto* btnEvt = std::get_if<event::MouseButtonEvent>(&event)) {
        if (handleMouseButton(*btnEvt)) {
            return true;
        }
    }

    // Mouse move (NEW - for dragging + hover)
    if (auto* moveEvt = std::get_if<event::MouseMoveEvent>(&event)) {
        if (handleMouseMove(*moveEvt)) {
            return true;
        }
    }

    // Forward to content
    if (content_) {
        auto viewport = getViewportRect();
        
        // Check if event is inside viewport
        if (auto* moveEvt = std::get_if<event::MouseMoveEvent>(&event)) {
            bool insideViewport = 
                moveEvt->position.x >= viewport.x && 
                moveEvt->position.x < static_cast<int32_t>(viewport.getRight()) &&
                moveEvt->position.y >= viewport.y && 
                moveEvt->position.y < static_cast<int32_t>(viewport.getBottom());
            
            if (insideViewport) {
                event::MouseMoveEvent transformed = *moveEvt;
                transformed.position = translateToContentSpace(moveEvt->position);
                return content_->onEvent(event::Event(transformed));
            }
        }
        
        if (auto* btnEvt = std::get_if<event::MouseButtonEvent>(&event)) {
            bool insideViewport = 
                btnEvt->position.x >= viewport.x && 
                btnEvt->position.x < static_cast<int32_t>(viewport.getRight()) &&
                btnEvt->position.y >= viewport.y && 
                btnEvt->position.y < static_cast<int32_t>(viewport.getBottom());
            
            if (insideViewport) {
                event::MouseButtonEvent transformed = *btnEvt;
                transformed.position = translateToContentSpace(btnEvt->position);
                return content_->onEvent(event::Event(transformed));
            }
        }
        
        return content_->onEvent(event);
    }

    return false;
}

// ============================================================================
// MOUSE EVENT HANDLERS (NEW)
// ============================================================================

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

bool ScrollView::handleMouseButton(const event::MouseButtonEvent& evt) {
    if (evt.button != event::MouseButtonEvent::Button::Left) return false;
    
    if (evt.action == event::MouseButtonEvent::Action::Press) {
        // Check if clicking on vertical scrollbar thumb
        auto vThumb = getVerticalThumbRect();
        if (evt.position.x >= vThumb.x && evt.position.x < static_cast<int32_t>(vThumb.getRight()) &&
            evt.position.y >= vThumb.y && evt.position.y < static_cast<int32_t>(vThumb.getBottom())) {
            draggingVScroll_ = true;
            dragStartPos_ = evt.position;
            dragStartOffset_ = scrollOffset_.y;
            return true;
        }
        
        // Check if clicking on horizontal scrollbar thumb
        auto hThumb = getHorizontalThumbRect();
        if (evt.position.x >= hThumb.x && evt.position.x < static_cast<int32_t>(hThumb.getRight()) &&
            evt.position.y >= hThumb.y && evt.position.y < static_cast<int32_t>(hThumb.getBottom())) {
            draggingHScroll_ = true;
            dragStartPos_ = evt.position;
            dragStartOffset_ = scrollOffset_.x;
            return true;
        }
        
        // Check if clicking on vertical scrollbar track (jump scroll)
        auto vScrollbar = getVerticalScrollbarRect();
        if (evt.position.x >= vScrollbar.x && evt.position.x < static_cast<int32_t>(vScrollbar.getRight()) &&
            evt.position.y >= vScrollbar.y && evt.position.y < static_cast<int32_t>(vScrollbar.getBottom())) {
            // Calculate target scroll position
            float ratio = static_cast<float>(evt.position.y - vScrollbar.y) / vScrollbar.h;
            auto viewport = getViewportRect();
            float maxScroll = std::max(0.0f, static_cast<float>(contentSize_.h - viewport.h));
            scrollTo(scrollOffset_.x, ratio * maxScroll);
            return true;
        }
        
        // Check if clicking on horizontal scrollbar track
        auto hScrollbar = getHorizontalScrollbarRect();
        if (evt.position.x >= hScrollbar.x && evt.position.x < static_cast<int32_t>(hScrollbar.getRight()) &&
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
    
    // Update hover state for visual feedback
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
// HIT TESTING (NEW)
// ============================================================================

bool ScrollView::isPointInVerticalScrollbar(const Point<int32_t>& point) const {
    auto thumbRect = getVerticalThumbRect();
    return thumbRect.w > 0 && 
           point.x >= thumbRect.x && point.x < static_cast<int32_t>(thumbRect.getRight()) &&
           point.y >= thumbRect.y && point.y < static_cast<int32_t>(thumbRect.getBottom());
}

bool ScrollView::isPointInHorizontalScrollbar(const Point<int32_t>& point) const {
    auto thumbRect = getHorizontalThumbRect();
    return thumbRect.h > 0 && 
           point.x >= thumbRect.x && point.x < static_cast<int32_t>(thumbRect.getRight()) &&
           point.y >= thumbRect.y && point.y < static_cast<int32_t>(thumbRect.getBottom());
}

// ============================================================================
// RENDER & HELPERS (Unchanged)
// ============================================================================

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

    // Render content with clipping and translation
    auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer);
    if (!extRenderer) {
        content_->render(renderer);
        return;
    }

    renderer.pushClip(viewport);
    extRenderer->save();
    extRenderer->translate(-scrollOffset_.x, -scrollOffset_.y);
    
    content_->render(renderer);
    
    extRenderer->restore();
    renderer.popClip();

    // Render scrollbars on top
    renderScrollbars(renderer);
}

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

Rect<int32_t, uint32_t> ScrollView::getVerticalScrollbarRect() const {
    auto rect = getRect();
    auto viewport = getViewportRect();
    
    if (!showScrollbars_ || !verticalScrollEnabled_ || contentSize_.h <= viewport.h) {
        return Rect<int32_t, uint32_t>(0, 0, 0, 0);
    }
    
    int32_t x = rect.x + static_cast<int32_t>(viewport.w);
    return Rect<int32_t, uint32_t>(x, rect.y, static_cast<uint32_t>(scrollbarWidth_), viewport.h);
}

Rect<int32_t, uint32_t> ScrollView::getHorizontalScrollbarRect() const {
    auto rect = getRect();
    auto viewport = getViewportRect();
    
    if (!showScrollbars_ || !horizontalScrollEnabled_ || contentSize_.w <= viewport.w) {
        return Rect<int32_t, uint32_t>(0, 0, 0, 0);
    }
    
    int32_t y = rect.y + static_cast<int32_t>(viewport.h);
    return Rect<int32_t, uint32_t>(rect.x, y, viewport.w, static_cast<uint32_t>(scrollbarWidth_));
}

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

void ScrollView::renderScrollbars(Renderer& renderer) {
    if (!showScrollbars_) return;
    
    auto viewport = getViewportRect();
    
    // Vertical scrollbar
    if (verticalScrollEnabled_ && contentSize_.h > viewport.h) {
        auto scrollbarRect = getVerticalScrollbarRect();
        auto thumbRect = getVerticalThumbRect();
        
        // Track background
        renderer.fillRect(scrollbarRect, Color(240, 240, 240, 200));
        
        // Thumb (with hover effect)
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

Point<int32_t> ScrollView::translateToContentSpace(const Point<int32_t>& screenPoint) const {
    auto viewport = getViewportRect();
    return Point<int32_t>(
        screenPoint.x - viewport.x + static_cast<int32_t>(scrollOffset_.x),
        screenPoint.y - viewport.y + static_cast<int32_t>(scrollOffset_.y)
    );
}

} // namespace frqs::widget