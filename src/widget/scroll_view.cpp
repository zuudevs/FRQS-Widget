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
    
    // ✅ FIX 1: Panggil recheckHover saat dragging scrollbar
    recheckHover(); 
    
    invalidate();
}

void ScrollView::scrollBy(float dx, float dy) {
    scrollOffset_.x += dx;
    scrollOffset_.y += dy;
    clampScrollOffset();
    
    // Panggil recheckHover saat mouse wheel
    recheckHover();
    
    // Force invalidate content
    if (content_) {
        if (auto* contentWidget = dynamic_cast<Widget*>(content_.get())) {
            contentWidget->invalidate();
        }
    }
    
    invalidate();
}

void ScrollView::recheckHover() {
    if (!content_) return;

    // Buat event sintetik
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

    // ✅ FIX 2 (AGGRESSIVE FLUSH):
    // Kirim koordinat LIMBO dulu untuk memaksa RESET semua hover state.
    // Ini memperbaiki masalah "Ghost Hover" yang nyangkut.
    {
        event::MouseMoveEvent limboEvent = synthEvent;
        limboEvent.position = Point<int32_t>(-99999, -99999);
        event::Event evt = limboEvent;
        content_->onEvent(evt);
    }

    // Baru kirim koordinat ASLI jika mouse ada di dalam viewport
    if (insideViewport) {
        event::MouseMoveEvent realEvent = synthEvent;
        realEvent.position = translateToContentSpace(synthEvent.position);
        event::Event evt = realEvent;
        content_->onEvent(evt);
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
    // --- Mouse Wheel ---
    if (auto* wheelEvt = std::get_if<event::MouseWheelEvent>(&event)) {
        lastMousePos_ = wheelEvt->position; // Update posisi

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
        
        if (!onScrollbar && content_) {
            event::MouseWheelEvent transformed = *wheelEvt;
            transformed.position = translateToContentSpace(wheelEvt->position);
            event::Event contentEvent = transformed;
            return content_->onEvent(contentEvent);
        }
        return false;
    }

    // --- Mouse Button ---
    if (auto* btnEvt = std::get_if<event::MouseButtonEvent>(&event)) {
        lastMousePos_ = btnEvt->position; // Update posisi
        
        if (handleMouseButton(*btnEvt)) {
            return true;
        }
        
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
                event::Event contentEvent = transformed;
                return content_->onEvent(contentEvent);
            }
        }
        return false;
    }

    // --- Mouse Move ---
    if (auto* moveEvt = std::get_if<event::MouseMoveEvent>(&event)) {
        lastMousePos_ = moveEvt->position; // Update posisi
        
        // Handle dragging scrollbar
        if (handleMouseMove(*moveEvt)) {
            // Jika sedang dragging, kita skip kirim event ke konten
            // Biar recheckHover yang dipanggil oleh scrollTo yang mengurusnya
            return true;
        }
        
        if (content_) {
            auto viewport = getViewportRect();
            bool insideViewport = 
                moveEvt->position.x >= viewport.x && 
                moveEvt->position.x < viewport.getRight() &&
                moveEvt->position.y >= viewport.y && 
                moveEvt->position.y < viewport.getBottom();
            
            // ✅ FIX 3: Gunakan logika yang sama dengan recheckHover (Aggressive Flush)
            // 1. Kirim Limbo dulu untuk safety
            {
                event::MouseMoveEvent limbo = *moveEvt;
                limbo.position = Point<int32_t>(-99999, -99999);
                event::Event contentEvent = limbo;
                content_->onEvent(contentEvent);
            }

            // 2. Kirim Real jika di dalam
            if (insideViewport) {
                event::MouseMoveEvent transformed = *moveEvt;
                transformed.position = translateToContentSpace(moveEvt->position);
                transformed.delta = moveEvt->delta;
                event::Event contentEvent = transformed;
                return content_->onEvent(contentEvent);
            }
        }
        return false;
    }

    if (content_) {
        return content_->onEvent(event);
    }

    return false;
}

void ScrollView::render(Renderer& renderer) {
    if (!isVisible()) return;

    auto rect = getRect();
    auto viewport = getViewportRect();

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

    renderer.pushClip(viewport);
    extRenderer->save();
    extRenderer->translate(-scrollOffset_.x, -scrollOffset_.y);
    
    content_->render(renderer);
    
    extRenderer->restore();
    renderer.popClip();

    renderScrollbars(renderer);
}

// ... (Sisanya: updateContentSize, clampScrollOffset, getters, renderScrollbars dll COPY-PASTE dari file lama JANGAN DIHAPUS) ...

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
    
    if (verticalScrollEnabled_ && contentSize_.h > viewport.h) {
        renderer.fillRect(getVerticalScrollbarRect(), Color(240, 240, 240, 200));
        renderer.fillRect(getVerticalThumbRect(), hoveringVScroll_ ? scrollbarHoverColor_ : scrollbarColor_);
    }
    if (horizontalScrollEnabled_ && contentSize_.w > viewport.w) {
        renderer.fillRect(getHorizontalScrollbarRect(), Color(240, 240, 240, 200));
        renderer.fillRect(getHorizontalThumbRect(), hoveringHScroll_ ? scrollbarHoverColor_ : scrollbarColor_);
    }
}

bool ScrollView::handleMouseWheel(const event::MouseWheelEvent& evt) {
    auto viewport = getViewportRect();
    bool inside = evt.position.x >= viewport.x && 
                  evt.position.x < static_cast<int32_t>(viewport.getRight()) &&
                  evt.position.y >= viewport.y && 
                  evt.position.y < static_cast<int32_t>(viewport.getBottom());
    if (!inside) return false;
    
    scrollBy(0.0f, -static_cast<float>(evt.delta) / 4.0f);
    return true;
}

bool ScrollView::handleMouseButton(const event::MouseButtonEvent& evt) {
    if (evt.button != event::MouseButtonEvent::Button::Left) return false;
    
    if (evt.action == event::MouseButtonEvent::Action::Press) {
        auto vThumb = getVerticalThumbRect();
        auto hThumb = getHorizontalThumbRect();
        
        if (evt.position.x >= vThumb.x && evt.position.x < static_cast<int32_t>(vThumb.getRight()) &&
            evt.position.y >= vThumb.y && evt.position.y < static_cast<int32_t>(vThumb.getBottom())) {
            draggingVScroll_ = true;
            dragStartPos_ = evt.position;
            dragStartOffset_ = scrollOffset_.y;
            return true;
        }
        if (evt.position.x >= hThumb.x && evt.position.x < static_cast<int32_t>(hThumb.getRight()) &&
            evt.position.y >= hThumb.y && evt.position.y < static_cast<int32_t>(hThumb.getBottom())) {
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
    if (wasHoveringV != hoveringVScroll_ || wasHoveringH != hoveringHScroll_) invalidate();
    return false;
}

Point<int32_t> ScrollView::translateToContentSpace(const Point<int32_t>& screenPoint) const {
    auto viewport = getViewportRect();
    return Point<int32_t>(
        screenPoint.x - viewport.x + static_cast<int32_t>(scrollOffset_.x),
        screenPoint.y - viewport.y + static_cast<int32_t>(scrollOffset_.y)
    );
}

bool ScrollView::isPointInVerticalScrollbar(const Point<int32_t>& point) const {
    auto thumbRect = getVerticalThumbRect();
    return thumbRect.w > 0 && point.x >= thumbRect.x && point.x < static_cast<int32_t>(thumbRect.getRight()) &&
           point.y >= thumbRect.y && point.y < static_cast<int32_t>(thumbRect.getBottom());
}

bool ScrollView::isPointInHorizontalScrollbar(const Point<int32_t>& point) const {
    auto thumbRect = getHorizontalThumbRect();
    return thumbRect.h > 0 && point.x >= thumbRect.x && point.x < static_cast<int32_t>(thumbRect.getRight()) &&
           point.y >= thumbRect.y && point.y < static_cast<int32_t>(thumbRect.getBottom());
}

} // namespace frqs::widget