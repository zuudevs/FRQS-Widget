// include/widget/scroll_view.hpp - FIXED VERSION
#pragma once

#include "iwidget.hpp"
#include "render/renderer.hpp"
#include <memory>

namespace frqs::widget {

// ============================================================================
// SCROLL VIEW WIDGET (Viewport with scrollbars)
// ============================================================================

class ScrollView : public Widget {
public:
    ScrollView();
    ~ScrollView() override = default;

    // ========================================================================
    // CONTENT MANAGEMENT
    // ========================================================================

    void setContent(std::shared_ptr<IWidget> content);
    std::shared_ptr<IWidget> getContent() const noexcept { return content_; }

    // ========================================================================
    // SCROLLING CONTROL
    // ========================================================================

    void scrollTo(float x, float y);
    void scrollBy(float dx, float dy);
    void scrollToTop() { scrollTo(scrollOffset_.x, 0.0f); }
    void scrollToBottom();

    Point<float> getScrollOffset() const noexcept { return scrollOffset_; }
    Size<uint32_t> getContentSize() const noexcept { return contentSize_; }

    // ========================================================================
    // SCROLLBAR CONFIGURATION
    // ========================================================================

    void setShowScrollbars(bool show) noexcept { showScrollbars_ = show; }
    void setScrollbarWidth(float width) noexcept { scrollbarWidth_ = width; }
    void setVerticalScrollEnabled(bool enabled) noexcept { verticalScrollEnabled_ = enabled; }
    void setHorizontalScrollEnabled(bool enabled) noexcept { horizontalScrollEnabled_ = enabled; }

    bool isShowingScrollbars() const noexcept { return showScrollbars_; }
    bool isVerticalScrollEnabled() const noexcept { return verticalScrollEnabled_; }
    bool isHorizontalScrollEnabled() const noexcept { return horizontalScrollEnabled_; }

    // ========================================================================
    // WIDGET OVERRIDES
    // ========================================================================

    void setRect(const Rect<int32_t, uint32_t>& rect) override;
    bool onEvent(const event::Event& event) override;
    void render(Renderer& renderer) override;

private:
    // Content
    std::shared_ptr<IWidget> content_;
    Point<float> scrollOffset_{0.0f, 0.0f};
    Size<uint32_t> contentSize_{0, 0};

    // Scrollbar settings
    bool showScrollbars_ = true;
    bool verticalScrollEnabled_ = true;
    bool horizontalScrollEnabled_ = true;
    float scrollbarWidth_ = 12.0f;

    // Scrollbar state (interaction)
    bool draggingVScroll_ = false;
    bool draggingHScroll_ = false;
    bool hoveringVScroll_ = false;
    bool hoveringHScroll_ = false;
    
    // ✅ FIX: Track mouse position in both spaces
    Point<int32_t> lastMouseScreenPos_{-1, -1};  // Screen/viewport coordinates
    Point<int32_t> lastMouseContentPos_{-1, -1}; // Content coordinates (after transform)
    
    Point<int32_t> dragStartPos_;
    float dragStartOffset_ = 0.0f;

    // Scrollbar colors
    Color scrollbarColor_ = Color(150, 150, 150, 180);
    Color scrollbarHoverColor_ = Color(120, 120, 120, 220);

    // ========================================================================
    // INTERNAL HELPERS
    // ========================================================================

    void updateContentSize();
    void clampScrollOffset();

    // Scrollbar geometry
    Rect<int32_t, uint32_t> getViewportRect() const;
    Rect<int32_t, uint32_t> getVerticalScrollbarRect() const;
    Rect<int32_t, uint32_t> getHorizontalScrollbarRect() const;
    Rect<int32_t, uint32_t> getVerticalThumbRect() const;
    Rect<int32_t, uint32_t> getHorizontalThumbRect() const;

    // Rendering
    void renderScrollbars(Renderer& renderer);

    // Event handling
    bool handleMouseWheel(const event::MouseWheelEvent& evt);
    bool handleMouseButton(const event::MouseButtonEvent& evt);
    bool handleMouseMove(const event::MouseMoveEvent& evt);

    Point<int32_t> translateToContentSpace(const Point<int32_t>& screenPoint) const;
	void recheckHover();

    // Hit testing
    bool isPointInVerticalScrollbar(const Point<int32_t>& point) const;
    bool isPointInHorizontalScrollbar(const Point<int32_t>& point) const;
};

} // namespace frqs::widget