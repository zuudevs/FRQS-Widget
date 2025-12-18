// src/widget/scroll_view.cpp - FIXED: Hover synchronization after scroll
#include "widget/scroll_view.hpp"
#include "render/renderer.hpp"
#include <algorithm>
#include <cmath>
#include <chrono>

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
    
    // ✅ FIX: Update hover state after scroll
    recheckHover(); 
    
    invalidate();
}

void ScrollView::scrollBy(float dx, float dy) {
    scrollOffset_.x += dx;
    scrollOffset_.y += dy;
    clampScrollOffset();
    
    // ✅ FIX: Update hover state after scroll
    recheckHover();
    
    invalidate();
}

// ✅ NEW: Improved hover rechecking after scroll
void ScrollView::recheckHover() {
    if (!content_) return;
    
    // Check if we have a valid last mouse position
    if (lastMouseScreenPos_.x < 0 || lastMouseScreenPos_.y < 0) {
        return; // No mouse position recorded yet
    }
    
    auto viewport = getViewportRect();
    
    // Check if mouse is inside viewport
    bool insideViewport = 
        lastMouseScreenPos_.x >= viewport.x && 
        lastMouseScreenPos_.x < static_cast<int32_t>(viewport.getRight()) &&
        lastMouseScreenPos_.y >= viewport.y && 
        lastMouseScreenPos_.y < static_cast<int32_t>(viewport.getBottom());
    
    if (!insideViewport) {
        // Mouse is outside viewport, send "exit" event
        event::MouseMoveEvent exitEvent;
        exitEvent.position = Point<int32_t>(-99999, -99999);
        exitEvent.delta = Point<int32_t>(0, 0);
        exitEvent.modifiers = 0;
        exitEvent.timestamp = static_cast<uint64_t>(
            std::chrono::steady_clock::now().time_since_epoch().count()
        );
        
        event::Event evt = exitEvent;
        content_->onEvent(evt);
        return;
    }
    
    // ✅ CRITICAL FIX: Transform screen position to content space
    Point<int32_t> contentPos = translateToContentSpace(lastMouseScreenPos_);
    
    // Update content mouse position cache
    lastMouseContentPos_ = contentPos;
    
    // Create synthetic mouse move event in CONTENT SPACE
    event::MouseMoveEvent moveEvent;
    moveEvent.position = contentPos;  // ✅ Use transformed coordinates!
    moveEvent.delta = Point<int32_t>(0, 0);
    moveEvent.modifiers = 0;
    moveEvent.timestamp = static_cast<uint64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count()
    );
    
    event::Event evt = moveEvent;
    content_->onEvent(evt);
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
    // ========================================================================
    // MOUSE WHEEL EVENT
    // ========================================================================
    if (auto* wheelEvt = std::get_if<event::MouseWheelEvent>(&event)) {
        // ✅ FIX: Update tracked position
        lastMouseScreenPos_ = wheelEvt->position;
        
        auto vScrollbar = getVerticalScrollbarRect();
        auto hScrollbar = getHorizontalScrollbarRect();
        
        bool onScrollbar = 
            (wheelEvt->position.x >= vScrollbar.x && 
             wheelEvt->position.x < vScrollbar.getRight() &&
             wheelEvt->position.y >= vScrollbar.y && 
             wheelEvt->position.y < vScrollbar.getBottom()) ||
            (wheelEvt->position.x >= hScrollbar.x && 
             wheelEvt->position.x < hScrollbar.getRight() &&
             wheelEvt->position.y >= hScrollbar.y && 
             wheelEvt->position.y < hScrollbar.getBottom());
        
        if (handleMouseWheel(*wheelEvt)) {
            // ✅ Scroll was handled, hover will be rechecked in scrollBy()
            return true;
        }
        
        if (!onScrollbar && content_) {
            event::MouseWheelEvent transformed = *wheelEvt;
            transformed.position = translateToContentSpace(wheelEvt->position);
            event::Event contentEvent = transformed;
            return content_->onEvent(contentEvent);
        }
        return false;
    }

    // ========================================================================
    // MOUSE BUTTON EVENT
    // ========================================================================
    if (auto* btnEvt = std::get_if<event::MouseButtonEvent>(&event)) {
        // ✅ FIX: Update tracked position
        lastMouseScreenPos_ = btnEvt->position;
        
        // Handle scrollbar interaction first
        if (handleMouseButton(*btnEvt)) {
            return true;
        }
        
        // Forward to content if inside viewport
        if (content_) {
            auto viewport = getViewportRect();
            bool insideViewport = 
                btnEvt->position.x >= viewport.x && 
                btnEvt->position.x < viewport.getRight() &&
                btnEvt->position.y >= viewport.y && 
                btnEvt->position.y < viewport.getBottom();
            
            if (insideViewport) {
                event::MouseButtonEvent transformed = *btnEvt;
                transformed.position = translateToContentSpace(btnEvt->position);
                lastMouseContentPos_ = transformed.position; // ✅ Update content position
                
                event::Event contentEvent = transformed;
                return content_->onEvent(contentEvent);
            }
        }
        return false;
    }

    // ========================================================================
    // MOUSE MOVE EVENT
    // ========================================================================
    if (auto* moveEvt = std::get_if<event::MouseMoveEvent>(&event)) {
        // ✅ FIX: Update tracked position
        lastMouseScreenPos_ = moveEvt->position;
        
        // Handle scrollbar dragging
        if (handleMouseMove(*moveEvt)) {
            // Dragging scrollbar, hover was already updated in scrollTo()
            return true;
        }
        
        // Forward to content
        if (content_) {
            auto viewport = getViewportRect();
            bool insideViewport = 
                moveEvt->position.x >= viewport.x && 
                moveEvt->position.x < viewport.getRight() &&
                moveEvt->position.y >= viewport.y && 
                moveEvt->position.y < viewport.getBottom();
            
            if (insideViewport) {
                // ✅ Transform to content space
                event::MouseMoveEvent transformed = *moveEvt;
                transformed.position = translateToContentSpace(moveEvt->position);
                lastMouseContentPos_ = transformed.position; // ✅ Update content position
                
                // Keep original delta (relative movement)
                transformed.delta = moveEvt->delta;
                
                event::Event contentEvent = transformed;
                return content_->onEvent(contentEvent);
            } else {
                // ✅ Mouse left viewport, send "exit" event
                event::MouseMoveEvent exitEvent = *moveEvt;
                exitEvent.position = Point<int32_t>(-99999, -99999);
                event::Event contentEvent = exitEvent;
                content_->onEvent(contentEvent);
                return false;
            }
        }
        return false;
    }

    // Other events - forward to content
    if (content_) {
        return content_->onEvent(event);
    }

    return false;
}

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

    auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer);
    if (!extRenderer) {
        content_->render(renderer);
        return;
    }

    // Render content with clipping and transform
    renderer.pushClip(viewport);
    extRenderer->save();
    extRenderer->translate(-scrollOffset_.x, -scrollOffset_.y);
    
    content_->render(renderer);
    
    extRenderer->restore();
    renderer.popClip();

    // Render scrollbars on top
    renderScrollbars(renderer);
}

// ============================================================================
// INTERNAL HELPERS
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
    uint32_t w = rect.w;
    uint32_t h = rect.h;
    
    if (showScrollbars_ && verticalScrollEnabled_ && contentSize_.h > rect.h) {
        w = w > static_cast<uint32_t>(scrollbarWidth_) ? 
            w - static_cast<uint32_t>(scrollbarWidth_) : 0;
    }
    if (showScrollbars_ && horizontalScrollEnabled_ && contentSize_.w > rect.w) {
        h = h > static_cast<uint32_t>(scrollbarWidth_) ? 
            h - static_cast<uint32_t>(scrollbarWidth_) : 0;
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
    return Rect<int32_t, uint32_t>(
        x, rect.y, 
        static_cast<uint32_t>(scrollbarWidth_), 
        viewport.h
    );
}

Rect<int32_t, uint32_t> ScrollView::getHorizontalScrollbarRect() const {
    auto rect = getRect();
    auto viewport = getViewportRect();
    
    if (!showScrollbars_ || !horizontalScrollEnabled_ || contentSize_.w <= viewport.w) {
        return Rect<int32_t, uint32_t>(0, 0, 0, 0);
    }
    
    int32_t y = rect.y + static_cast<int32_t>(viewport.h);
    return Rect<int32_t, uint32_t>(
        rect.x, y, 
        viewport.w, 
        static_cast<uint32_t>(scrollbarWidth_)
    );
}

Rect<int32_t, uint32_t> ScrollView::getVerticalThumbRect() const {
    auto scrollbarRect = getVerticalScrollbarRect();
    if (scrollbarRect.w == 0) return scrollbarRect;
    
    auto viewport = getViewportRect();
    float ratio = static_cast<float>(viewport.h) / contentSize_.h;
    uint32_t thumbHeight = std::max(
        static_cast<uint32_t>(scrollbarRect.h * ratio), 
        20u
    );
    
    float maxScroll = std::max(
        0.0f, 
        static_cast<float>(contentSize_.h - viewport.h)
    );
    float scrollRatio = maxScroll > 0.0f ? (scrollOffset_.y / maxScroll) : 0.0f;
    
    int32_t thumbY = scrollbarRect.y + static_cast<int32_t>(
        (scrollbarRect.h - thumbHeight) * scrollRatio
    );
    
    return Rect<int32_t, uint32_t>(
        scrollbarRect.x, thumbY, 
        scrollbarRect.w, thumbHeight
    );
}

Rect<int32_t, uint32_t> ScrollView::getHorizontalThumbRect() const {
    auto scrollbarRect = getHorizontalScrollbarRect();
    if (scrollbarRect.h == 0) return scrollbarRect;
    
    auto viewport = getViewportRect();
    float ratio = static_cast<float>(viewport.w) / contentSize_.w;
    uint32_t thumbWidth = std::max(
        static_cast<uint32_t>(scrollbarRect.w * ratio), 
        20u
    );
    
    float maxScroll = std::max(
        0.0f, 
        static_cast<float>(contentSize_.w - viewport.w)
    );
    float scrollRatio = maxScroll > 0.0f ? (scrollOffset_.x / maxScroll) : 0.0f;
    
    int32_t thumbX = scrollbarRect.x + static_cast<int32_t>(
        (scrollbarRect.w - thumbWidth) * scrollRatio
    );
    
    return Rect<int32_t, uint32_t>(
        thumbX, scrollbarRect.y, 
        thumbWidth, scrollbarRect.h
    );
}

void ScrollView::renderScrollbars(Renderer& renderer) {
    if (!showScrollbars_) return;
    
    auto viewport = getViewportRect();
    
    // Vertical scrollbar
    if (verticalScrollEnabled_ && contentSize_.h > viewport.h) {
        renderer.fillRect(
            getVerticalScrollbarRect(), 
            Color(240, 240, 240, 200)
        );
        renderer.fillRect(
            getVerticalThumbRect(), 
            hoveringVScroll_ ? scrollbarHoverColor_ : scrollbarColor_
        );
    }
    
    // Horizontal scrollbar
    if (horizontalScrollEnabled_ && contentSize_.w > viewport.w) {
        renderer.fillRect(
            getHorizontalScrollbarRect(), 
            Color(240, 240, 240, 200)
        );
        renderer.fillRect(
            getHorizontalThumbRect(), 
            hoveringHScroll_ ? scrollbarHoverColor_ : scrollbarColor_
        );
    }
}

bool ScrollView::handleMouseWheel(const event::MouseWheelEvent& evt) {
    auto viewport = getViewportRect();
    
    bool inside = 
        evt.position.x >= viewport.x && 
        evt.position.x < static_cast<int32_t>(viewport.getRight()) &&
        evt.position.y >= viewport.y && 
        evt.position.y < static_cast<int32_t>(viewport.getBottom());
    
    if (!inside) return false;
    
    // Scroll by delta (negative delta = scroll down)
    scrollBy(0.0f, -static_cast<float>(evt.delta) / 4.0f);
    
    return true;
}

bool ScrollView::handleMouseButton(const event::MouseButtonEvent& evt) {
    if (evt.button != event::MouseButtonEvent::Button::Left) return false;
    
    if (evt.action == event::MouseButtonEvent::Action::Press) {
        auto vThumb = getVerticalThumbRect();
        auto hThumb = getHorizontalThumbRect();
        
        // Check vertical scrollbar thumb
        if (evt.position.x >= vThumb.x && 
            evt.position.x < static_cast<int32_t>(vThumb.getRight()) &&
            evt.position.y >= vThumb.y && 
            evt.position.y < static_cast<int32_t>(vThumb.getBottom())) {
            draggingVScroll_ = true;
            dragStartPos_ = evt.position;
            dragStartOffset_ = scrollOffset_.y;
            return true;
        }
        
        // Check horizontal scrollbar thumb
        if (evt.position.x >= hThumb.x && 
            evt.position.x < static_cast<int32_t>(hThumb.getRight()) &&
            evt.position.y >= hThumb.y && 
            evt.position.y < static_cast<int32_t>(hThumb.getBottom())) {
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
    // Handle vertical scrollbar dragging
    if (draggingVScroll_) {
        auto viewport = getViewportRect();
        auto scrollbarRect = getVerticalScrollbarRect();
        auto thumbRect = getVerticalThumbRect();
        
        int32_t deltaY = evt.position.y - dragStartPos_.y;
        float maxScroll = std::max(
            0.0f, 
            static_cast<float>(contentSize_.h - viewport.h)
        );
        uint32_t maxThumbOffset = scrollbarRect.h - thumbRect.h;
        
        if (maxThumbOffset > 0) {
            float scrollDelta = (static_cast<float>(deltaY) / maxThumbOffset) * maxScroll;
            scrollTo(scrollOffset_.x, dragStartOffset_ + scrollDelta);
        }
        return true;
    }
    
    // Handle horizontal scrollbar dragging
    if (draggingHScroll_) {
        auto viewport = getViewportRect();
        auto scrollbarRect = getHorizontalScrollbarRect();
        auto thumbRect = getHorizontalThumbRect();
        
        int32_t deltaX = evt.position.x - dragStartPos_.x;
        float maxScroll = std::max(
            0.0f, 
            static_cast<float>(contentSize_.w - viewport.w)
        );
        uint32_t maxThumbOffset = scrollbarRect.w - thumbRect.w;
        
        if (maxThumbOffset > 0) {
            float scrollDelta = (static_cast<float>(deltaX) / maxThumbOffset) * maxScroll;
            scrollTo(dragStartOffset_ + scrollDelta, scrollOffset_.y);
        }
        return true;
    }
    
    // Update scrollbar hover states
    bool wasHoveringV = hoveringVScroll_;
    bool wasHoveringH = hoveringHScroll_;
    
    hoveringVScroll_ = isPointInVerticalScrollbar(evt.position);
    hoveringHScroll_ = isPointInHorizontalScrollbar(evt.position);
    
    if (wasHoveringV != hoveringVScroll_ || wasHoveringH != hoveringHScroll_) {
        invalidate();
    }
    
    return false;
}

Point<int32_t> ScrollView::translateToContentSpace(
    const Point<int32_t>& screenPoint
) const {
    auto viewport = getViewportRect();
    
    return Point<int32_t>(
        screenPoint.x - viewport.x + static_cast<int32_t>(scrollOffset_.x),
        screenPoint.y - viewport.y + static_cast<int32_t>(scrollOffset_.y)
    );
}

bool ScrollView::isPointInVerticalScrollbar(const Point<int32_t>& point) const {
    auto thumbRect = getVerticalThumbRect();
    
    return thumbRect.w > 0 && 
           point.x >= thumbRect.x && 
           point.x < static_cast<int32_t>(thumbRect.getRight()) &&
           point.y >= thumbRect.y && 
           point.y < static_cast<int32_t>(thumbRect.getBottom());
}

bool ScrollView::isPointInHorizontalScrollbar(const Point<int32_t>& point) const {
    auto thumbRect = getHorizontalThumbRect();
    
    return thumbRect.h > 0 && 
           point.x >= thumbRect.x && 
           point.x < static_cast<int32_t>(thumbRect.getRight()) &&
           point.y >= thumbRect.y && 
           point.y < static_cast<int32_t>(thumbRect.getBottom());
}

} // namespace frqs::widget