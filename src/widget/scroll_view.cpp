// src/widget/scroll_view.cpp - FINAL FIXED VERSION (Report 7 Complete Fix)
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
    invalidate();
}

void ScrollView::scrollBy(float dx, float dy) {
    scrollOffset_.x += dx;
    scrollOffset_.y += dy;
    clampScrollOffset();
    
    // ✅ FIX: Recheck hover to update button states after scroll
    recheckHover();
    
    // ✅ FIX: Force invalidate content to ensure all children re-render with updated states
    if (content_) {
        if (auto* contentWidget = dynamic_cast<Widget*>(content_.get())) {
            contentWidget->invalidate();
        }
    }
    
    invalidate();
}

void ScrollView::recheckHover() {
    // ✅ FIX Report 7: Create complete synthetic event with valid timestamp
    // This ensures consistency with real MouseMove events from Windows
    event::MouseMoveEvent synthEvent;
    synthEvent.position = lastMousePos_; 
    synthEvent.delta = Point<int32_t>(0, 0); 
    synthEvent.modifiers = 0; 
    synthEvent.timestamp = static_cast<uint64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count()
    );

    auto viewport = getViewportRect();
    bool insideViewport = 
        lastMousePos_.x >= viewport.x && 
        lastMousePos_.x < static_cast<int32_t>(viewport.getRight()) &&
        lastMousePos_.y >= viewport.y && 
        lastMousePos_.y < static_cast<int32_t>(viewport.getBottom());

    if (insideViewport && content_) {
        // ✅ FIX Report 7: Transform coordinates to content space
        event::MouseMoveEvent transformed = synthEvent;
        transformed.position = translateToContentSpace(synthEvent.position);
        
        // ✅ FIX Report 7: Explicitly wrap in Event variant for proper propagation
        // This ensures the event is handled consistently with real events
        event::Event contentEvent = transformed;
        content_->onEvent(contentEvent);
    }
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
    if (auto* wheelEvt = std::get_if<event::MouseWheelEvent>(&event)) {
        auto vScrollbar = getVerticalScrollbarRect();
        auto hScrollbar = getHorizontalScrollbarRect();
        
        bool onScrollbar = 
            (wheelEvt->position.x >= vScrollbar.x && wheelEvt->position.x < vScrollbar.getRight() &&
             wheelEvt->position.y >= vScrollbar.y && wheelEvt->position.y < vScrollbar.getBottom()) ||
            (wheelEvt->position.x >= hScrollbar.x && wheelEvt->position.x < hScrollbar.getRight() &&
             wheelEvt->position.y >= hScrollbar.y && wheelEvt->position.y < hScrollbar.getBottom());
        
        if (handleMouseWheel(*wheelEvt)) {
            return true;
        }
        
        // Pass to content with transformed coordinates
        if (!onScrollbar && content_) {
            event::MouseWheelEvent transformed = *wheelEvt;
            transformed.position = translateToContentSpace(wheelEvt->position);
            event::Event contentEvent = transformed;
            return content_->onEvent(contentEvent);
        }
        return false;
    }

    // Mouse button (for scrollbar dragging)
    if (auto* btnEvt = std::get_if<event::MouseButtonEvent>(&event)) {
        // ✅ FIX: Update lastMousePos for button events
        lastMousePos_ = btnEvt->position;
        
        // Check scrollbar interaction first
        if (handleMouseButton(*btnEvt)) {
            return true;
        }
        
        // ✅ CRITICAL FIX: Transform coordinates before passing to content
        if (content_) {
            auto viewport = getViewportRect();
            
            // Check if click is inside viewport bounds
            bool insideViewport = 
                btnEvt->position.x >= viewport.x && 
                btnEvt->position.x < viewport.getRight() &&
                btnEvt->position.y >= viewport.y && 
                btnEvt->position.y < viewport.getBottom();
            
            if (insideViewport) {
                // Transform to content space
                event::MouseButtonEvent transformed = *btnEvt;
                transformed.position = translateToContentSpace(btnEvt->position);
                event::Event contentEvent = transformed;
                return content_->onEvent(contentEvent);
            }
        }
        return false;
    }

    // Mouse move (for scrollbar hover and content interaction)
    if (auto* moveEvt = std::get_if<event::MouseMoveEvent>(&event)) {
        // ✅ FIX: Always update lastMousePos on mouse move
        lastMousePos_ = moveEvt->position;
        
        // Handle scrollbar interaction first
        if (handleMouseMove(*moveEvt)) {
            return true;
        }
        
        // ✅ CRITICAL FIX: Transform coordinates before passing to content
        if (content_) {
            auto viewport = getViewportRect();
            
            // Check if mouse is inside viewport
            bool insideViewport = 
                moveEvt->position.x >= viewport.x && 
                moveEvt->position.x < viewport.getRight() &&
                moveEvt->position.y >= viewport.y && 
                moveEvt->position.y < viewport.getBottom();
            
            if (insideViewport) {
                // Transform to content space
                event::MouseMoveEvent transformed = *moveEvt;
                transformed.position = translateToContentSpace(moveEvt->position);
                // Delta remains unchanged (relative movement)
                transformed.delta = moveEvt->delta;
                event::Event contentEvent = transformed;
                return content_->onEvent(contentEvent);
            }
        }
        return false;
    }

    // For other events (keyboard, etc.), pass directly to content
    if (content_) {
        return content_->onEvent(event);
    }

    return false;
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

    auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer);
    if (!extRenderer) {
        content_->render(renderer);
        return;
    }

    // ========================================================================
    // RENDER CONTENT WITH TRANSFORM
    // ========================================================================

    renderer.pushClip(viewport);
    extRenderer->save();
    extRenderer->translate(-scrollOffset_.x, -scrollOffset_.y);
    
    content_->render(renderer);
    
    extRenderer->restore();
    renderer.popClip();

    // ========================================================================
    // RENDER SCROLLBARS (NO TRANSFORM)
    // ========================================================================

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
    float ratio = static_cast<float>(viewport.h) / contentSize_.h;
    uint32_t thumbHeight = static_cast<uint32_t>(scrollbarRect.h * ratio);
    thumbHeight = std::max(thumbHeight, 20u);

    float maxScroll = std::max(0.0f, static_cast<float>(contentSize_.h - viewport.h));
    float scrollRatio = maxScroll > 0.0f ? (scrollOffset_.y / maxScroll) : 0.0f;
    uint32_t maxThumbOffset = scrollbarRect.h - thumbHeight;
    int32_t thumbY = scrollbarRect.y + static_cast<int32_t>(maxThumbOffset * scrollRatio);

    return Rect<int32_t, uint32_t>(scrollbarRect.x, thumbY, scrollbarRect.w, thumbHeight);
}

Rect<int32_t, uint32_t> ScrollView::getHorizontalThumbRect() const {
    auto scrollbarRect = getHorizontalScrollbarRect();
    if (scrollbarRect.h == 0) return scrollbarRect;

    auto viewport = getViewportRect();
    float ratio = static_cast<float>(viewport.w) / contentSize_.w;
    uint32_t thumbWidth = static_cast<uint32_t>(scrollbarRect.w * ratio);
    thumbWidth = std::max(thumbWidth, 20u);

    float maxScroll = std::max(0.0f, static_cast<float>(contentSize_.w - viewport.w));
    float scrollRatio = maxScroll > 0.0f ? (scrollOffset_.x / maxScroll) : 0.0f;
    uint32_t maxThumbOffset = scrollbarRect.w - thumbWidth;
    int32_t thumbX = scrollbarRect.x + static_cast<int32_t>(maxThumbOffset * scrollRatio);

    return Rect<int32_t, uint32_t>(thumbX, scrollbarRect.y, thumbWidth, scrollbarRect.h);
}

// ============================================================================
// RENDERING
// ============================================================================

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

// ============================================================================
// EVENT HANDLING
// ============================================================================

bool ScrollView::handleMouseWheel(const event::MouseWheelEvent& evt) {
    auto viewport = getViewportRect();

    bool inside = evt.position.x >= viewport.x && 
                  evt.position.x < static_cast<int32_t>(viewport.getRight()) &&
                  evt.position.y >= viewport.y && 
                  evt.position.y < static_cast<int32_t>(viewport.getBottom());

    if (!inside) return false;

    // ✅ FIX: Update lastMousePos before scrolling
    lastMousePos_ = evt.position;

    float delta = static_cast<float>(evt.delta);
    float scrollAmount = -delta / 4.0f;

    scrollBy(0.0f, scrollAmount);

    return true;
}

bool ScrollView::handleMouseButton(const event::MouseButtonEvent& evt) {
    if (evt.button != event::MouseButtonEvent::Button::Left) {
        return false;
    }

    if (evt.action == event::MouseButtonEvent::Action::Press) {
        auto vThumb = getVerticalThumbRect();
        auto hThumb = getHorizontalThumbRect();

        bool inVThumb = evt.position.x >= vThumb.x && 
                        evt.position.x < static_cast<int32_t>(vThumb.getRight()) &&
                        evt.position.y >= vThumb.y && 
                        evt.position.y < static_cast<int32_t>(vThumb.getBottom());

        bool inHThumb = evt.position.x >= hThumb.x && 
                        evt.position.x < static_cast<int32_t>(hThumb.getRight()) &&
                        evt.position.y >= hThumb.y && 
                        evt.position.y < static_cast<int32_t>(hThumb.getBottom());

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
// ✅ COORDINATE TRANSFORMATION (The Core Fix for Report 7)
// ============================================================================

Point<int32_t> ScrollView::translateToContentSpace(const Point<int32_t>& screenPoint) const {
    auto viewport = getViewportRect();

    // Formula: ContentSpace = ScreenSpace - ViewportOrigin + ScrollOffset
    return Point<int32_t>(
        screenPoint.x - viewport.x + static_cast<int32_t>(scrollOffset_.x),
        screenPoint.y - viewport.y + static_cast<int32_t>(scrollOffset_.y)
    );
}

// ============================================================================
// HIT TESTING
// ============================================================================

bool ScrollView::isPointInVerticalScrollbar(const Point<int32_t>& point) const {
    auto thumbRect = getVerticalThumbRect();
    if (thumbRect.w == 0) return false;

    return point.x >= thumbRect.x && 
           point.x < static_cast<int32_t>(thumbRect.getRight()) &&
           point.y >= thumbRect.y && 
           point.y < static_cast<int32_t>(thumbRect.getBottom());
}

bool ScrollView::isPointInHorizontalScrollbar(const Point<int32_t>& point) const {
    auto thumbRect = getHorizontalThumbRect();
    if (thumbRect.h == 0) return false;

    return point.x >= thumbRect.x && 
           point.x < static_cast<int32_t>(thumbRect.getRight()) &&
           point.y >= thumbRect.y && 
           point.y < static_cast<int32_t>(thumbRect.getBottom());
}

} // namespace frqs::widget