/**
 * @file scroll_view.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines the ScrollView widget, a container that provides a scrollable viewport for a single child widget.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "iwidget.hpp"
#include <memory>

namespace frqs::widget {

// ============================================================================
// SCROLL VIEW WIDGET (Viewport with draggable scrollbars)
// ============================================================================

/**
 * @brief A widget that provides a scrollable view of a single, larger child widget.
 * @details The ScrollView acts as a viewport, managing vertical and horizontal scrollbars
 * to allow navigation of a content widget that is larger than the ScrollView's own bounds.
 * It handles mouse wheel events and scrollbar drag-and-drop interactions.
 */
class ScrollView : public Widget {
public:
    /**
     * @brief Constructs a new ScrollView object.
     */
    ScrollView();
    /**
     * @brief Default destructor.
     */
    ~ScrollView() override = default;

    // ========================================================================
    // CONTENT MANAGEMENT
    // ========================================================================

    /**
     * @brief Sets the content widget to be displayed within the scroll view.
     * @param[in] content A shared pointer to the widget that will be scrollable.
     */
    void setContent(std::shared_ptr<IWidget> content);

    /**
     * @brief Gets the currently set content widget.
     * @return A shared pointer to the content widget.
     */
    std::shared_ptr<IWidget> getContent() const noexcept { return content_; }

    // ========================================================================
    // SCROLLING CONTROL
    // ========================================================================

    /**
     * @brief Scrolls the view to an absolute offset.
     * @param[in] x The horizontal scroll position.
     * @param[in] y The vertical scroll position.
     */
    void scrollTo(float x, float y);

    /**
     * @brief Scrolls the view by a relative amount.
     * @param[in] dx The change in horizontal scroll position.
     * @param[in] dy The change in vertical scroll position.
     */
    void scrollBy(float dx, float dy);

    /**
     * @brief Scrolls the view to the top (vertical position 0).
     */
    void scrollToTop() { scrollTo(scrollOffset_.x, 0.0f); }

    /**
     * @brief Scrolls the view to the bottom.
     */
    void scrollToBottom();

	/**
     * @brief Performs a hit test, forwarding it to the content in the correct coordinate space.
     * @param[in] point The point to test, in the ScrollView's coordinate system.
     * @return A pointer to the widget that was hit, or `nullptr` if no widget was hit.
     */
	IWidget* hitTest(const Point<int32_t>& point) override ;

    /**
     * @brief Gets the current scroll offset.
     * @return A Point representing the (x, y) scroll offset.
     */
    Point<float> getScrollOffset() const noexcept { return scrollOffset_; }

    /**
     * @brief Gets the last known size of the content widget.
     * @return The size of the content.
     */
    Size<uint32_t> getContentSize() const noexcept { return contentSize_; }

    // ========================================================================
    // SCROLLBAR CONFIGURATION
    // ========================================================================

    /**
     * @brief Sets whether scrollbars are shown when needed.
     * @param[in] show True to show scrollbars, false to hide them.
     */
    void setShowScrollbars(bool show) noexcept { showScrollbars_ = show; }

    /**
     * @brief Sets the width of the scrollbars.
     * @param[in] width The width in pixels.
     */
    void setScrollbarWidth(float width) noexcept { scrollbarWidth_ = width; }

    /**
     * @brief Enables or disables vertical scrolling.
     * @param[in] enabled True to enable, false to disable.
     */
    void setVerticalScrollEnabled(bool enabled) noexcept { verticalScrollEnabled_ = enabled; }

    /**
     * @brief Enables or disables horizontal scrolling.
     * @param[in] enabled True to enable, false to disable.
     */
    void setHorizontalScrollEnabled(bool enabled) noexcept { horizontalScrollEnabled_ = enabled; }

    /**
     * @brief Checks if scrollbars are configured to be shown.
     * @return True if scrollbars are shown, false otherwise.
     */
    bool isShowingScrollbars() const noexcept { return showScrollbars_; }

    /**
     * @brief Checks if vertical scrolling is enabled.
     * @return True if vertical scrolling is enabled, false otherwise.
     */
    bool isVerticalScrollEnabled() const noexcept { return verticalScrollEnabled_; }

    /**
     * @brief Checks if horizontal scrolling is enabled.
     * @return True if horizontal scrolling is enabled, false otherwise.
     */
    bool isHorizontalScrollEnabled() const noexcept { return horizontalScrollEnabled_; }

    // ========================================================================
    // WIDGET OVERRIDES
    // ========================================================================

    void setRect(const Rect<int32_t, uint32_t>& rect) override;
    bool onEvent(const event::Event& event) override;
    void render(Renderer& renderer) override;

private:
    // Content
    std::shared_ptr<IWidget> content_;          ///< The scrollable child widget.
    Point<float> scrollOffset_{0.0f, 0.0f};   ///< The current (x, y) scroll offset of the content.
    Size<uint32_t> contentSize_{0, 0};        ///< The last measured size of the content widget.

    // Scrollbar settings
    bool showScrollbars_ = true;              ///< If false, scrollbars are never shown.
    bool verticalScrollEnabled_ = true;       ///< If false, vertical scrolling is disabled.
    bool horizontalScrollEnabled_ = true;     ///< If false, horizontal scrolling is disabled.
    float scrollbarWidth_ = 12.0f;            ///< The width of the scrollbar tracks.

    // Scrollbar state
    bool draggingVScroll_ = false;            ///< True if the user is dragging the vertical scroll thumb.
    bool draggingHScroll_ = false;            ///< True if the user is dragging the horizontal scroll thumb.
    bool hoveringVScroll_ = false;            ///< True if the mouse is hovering over the vertical scrollbar.
    bool hoveringHScroll_ = false;            ///< True if the mouse is hovering over the horizontal scrollbar.
    Point<int32_t> dragStartPos_;             ///< Mouse position where a scrollbar drag started.
    float dragStartOffset_ = 0.0f;            ///< Scroll offset when a drag started.

    // Scrollbar colors
    Color scrollbarColor_ = Color(150, 150, 150, 180);       ///< Default color of the scrollbar thumbs.
    Color scrollbarHoverColor_ = Color(120, 120, 120, 220);  ///< Color of scrollbar thumbs when hovered.

    // ========================================================================
    // INTERNAL HELPERS
    // ========================================================================

    /**
     * @brief Updates the cached size of the content and recalculates scroll limits.
     */
    void updateContentSize();

    /**
     * @brief Ensures the scroll offset is within valid bounds [0, max_scroll].
     */
    void clampScrollOffset();

    // Scrollbar geometry
    /**
     * @brief Gets the rectangle of the content area, excluding scrollbars.
     * @return The viewport rectangle.
     */
    Rect<int32_t, uint32_t> getViewportRect() const;

    /**
     * @brief Gets the rectangle for the vertical scrollbar track.
     * @return The vertical scrollbar's rectangle.
     */
    Rect<int32_t, uint32_t> getVerticalScrollbarRect() const;

    /**
     * @brief Gets the rectangle for the horizontal scrollbar track.
     * @return The horizontal scrollbar's rectangle.
     */
    Rect<int32_t, uint32_t> getHorizontalScrollbarRect() const;

    /**
     * @brief Gets the rectangle for the vertical scrollbar's draggable thumb.
     * @return The vertical thumb's rectangle.
     */
    Rect<int32_t, uint32_t> getVerticalThumbRect() const;

    /**
     * @brief Gets the rectangle for the horizontal scrollbar's draggable thumb.
     * @return The horizontal thumb's rectangle.
     */
    Rect<int32_t, uint32_t> getHorizontalThumbRect() const;

    // Rendering
    /**
     * @brief Renders the vertical and horizontal scrollbars if needed.
     * @param[in,out] renderer The renderer to draw with.
     */
    void renderScrollbars(Renderer& renderer);

    // Event handling
    /**
     * @brief Handles mouse wheel events for scrolling.
     * @param[in] evt The mouse wheel event.
     * @return True if the event was handled.
     */
    bool handleMouseWheel(const event::MouseWheelEvent& evt);

    /**
     * @brief Handles mouse button events for scrollbar interaction.
     * @param[in] evt The mouse button event.
     * @return True if the event was handled.
     */
    bool handleMouseButton(const event::MouseButtonEvent& evt);

    /**
     * @brief Handles mouse move events for scrollbar dragging and hover states.
     * @param[in] evt The mouse move event.
     * @return True if the event was handled.
     */
    bool handleMouseMove(const event::MouseMoveEvent& evt);

    /**
     * @brief Translates a point from the ScrollView's coordinate space to the content's coordinate space.
     * @param[in] screenPoint A point relative to the ScrollView's origin.
     * @return The corresponding point relative to the content's origin.
     */
    Point<int32_t> translateToContentSpace(const Point<int32_t>& screenPoint) const;

    // Hit testing
    /**
     * @brief Checks if a point is within the vertical scrollbar's area.
     * @param[in] point The point to test.
     * @return True if the point is within the vertical scrollbar.
     */
    bool isPointInVerticalScrollbar(const Point<int32_t>& point) const;

    /**
     * @brief Checks if a point is within the horizontal scrollbar's area.
     * @param[in] point The point to test.
     * @return True if the point is within the horizontal scrollbar.
     */
    bool isPointInHorizontalScrollbar(const Point<int32_t>& point) const;
};

} // namespace frqs::widget