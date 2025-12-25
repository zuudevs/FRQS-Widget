/**
 * @file list_view.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Implements the ListView widget for displaying large, scrollable lists of items efficiently.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "widget/list_view.hpp"
#include "render/renderer.hpp"
#include <algorithm>

namespace frqs::widget {

// ============================================================================
// PIMPL
// ============================================================================

/**
 * @brief Private implementation details for the ListView.
 * @struct ListView::Impl
 * @internal
 */
struct ListView::Impl {
    // Reserved for future use
};

// ============================================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================================

/**
 * @brief Constructs a new ListView widget.
 */
ListView::ListView()
    : Widget()
    , pImpl_(std::make_unique<Impl>())
{
    setBackgroundColor(colors::White);
}

/**
 * @brief Destroys the ListView widget.
 */
ListView::~ListView() = default;

// ============================================================================
// ADAPTER MANAGEMENT
// ============================================================================

/**
 * @brief Sets the data adapter for the list view.
 * 
 * The adapter provides the data and views for the items in the list. Setting a
 * new adapter will clear the existing items and repopulate the list.
 * 
 * @param adapter A shared pointer to an object implementing the `IListAdapter` interface.
 */
void ListView::setAdapter(std::shared_ptr<IListAdapter> adapter) {
    adapter_ = std::move(adapter);
    
    // Clear pool
    widgetPool_.clear();
    
    // Reset state
    firstVisibleIndex_ = 0;
    lastVisibleIndex_ = 0;
    scrollOffset_ = 0.0f;
    selectedIndex_ = size_t(-1);
    
    if (adapter_) {
        calculateVisibleRange();
        updateWidgetPool();
    }
    
    invalidate();
}

/**
 * @brief Notifies the list view that the underlying data in the adapter has changed.
 * 
 * This should be called whenever the data set is modified (e.g., items added,
 * removed, or updated) to trigger a refresh of the visible items.
 */
void ListView::notifyDataChanged() {
    if (!adapter_) return;
    
    calculateVisibleRange();
    updateWidgetPool();
    invalidate();
}

// ========================================================================
// CONFIGURATION
// ========================================================================

/**
 * @brief Sets the height for each item in the list.
 * @param height The height of an item in pixels.
 */
void ListView::setItemHeight(uint32_t height) noexcept {
    if (itemHeight_ == height) return;
    itemHeight_ = height;
    notifyDataChanged();
}

/**
 * @brief Sets the vertical spacing between items in the list.
 * @param spacing The spacing in pixels.
 */
void ListView::setItemSpacing(uint32_t spacing) noexcept {
    if (itemSpacing_ == spacing) return;
    itemSpacing_ = spacing;
    notifyDataChanged();
}

// ========================================================================
// SELECTION
// ========================================================================

/**
 * @brief Sets the currently selected item by its index.
 * 
 * If the index is out of bounds, the selection is cleared.
 * If the selection changes, the `onSelectionChanged` callback is invoked.
 * 
 * @param index The index of the item to select.
 */
void ListView::setSelectedIndex(size_t index) {
    if (!adapter_ || index >= adapter_->getCount()) {
        selectedIndex_ = size_t(-1);
        return;
    }
    
    if (selectedIndex_ == index) return;
    
    selectedIndex_ = index;
    
    if (onSelectionChanged_) {
        onSelectionChanged_(index);
    }
    
    // Ensure selected item is visible
    scrollTo(index);
    
    invalidate();
}

// ========================================================================
// SCROLLING
// ========================================================================

/**
 * @brief Scrolls the list to make the item at the given index visible.
 * @param index The index of the item to scroll to.
 */
void ListView::scrollTo(size_t index) {
    if (!adapter_ || index >= adapter_->getCount()) return;
    
    float targetOffset = static_cast<float>(index * (itemHeight_ + itemSpacing_));
    scrollOffset_ = targetOffset;
    clampScrollOffset();
    
    calculateVisibleRange();
    updateWidgetPool();
    invalidate();
}

/**
 * @brief Scrolls the list to the very bottom.
 */
void ListView::scrollToBottom() {
    if (!adapter_) return;
    
    scrollOffset_ = getMaxScrollOffset();
    calculateVisibleRange();
    updateWidgetPool();
    invalidate();
}

/**
 * @brief Scrolls the list by a given amount.
 * @param delta The amount to scroll by. A negative value scrolls down, a positive value scrolls up.
 */
void ListView::scrollBy(float delta) {
    scrollOffset_ += delta;
    clampScrollOffset();
    
    calculateVisibleRange();
    updateWidgetPool();
    invalidate();
}

// ========================================================================
// WIDGET OVERRIDES
// ========================================================================

/**
 * @brief Sets the rectangle defining the widget's bounds.
 * @param rect The new widget bounds.
 */
void ListView::setRect(const Rect<int32_t, uint32_t>& rect) {
    Widget::setRect(rect);
    calculateVisibleRange();
    updateWidgetPool();
}

/**
 * @brief Handles incoming events for the widget.
 * @param event The event to process.
 * @return `true` if the event was handled, `false` otherwise.
 */
bool ListView::onEvent(const event::Event& event) {
    // Mouse wheel
    if (auto* wheelEvt = std::get_if<event::MouseWheelEvent>(&event)) {
        if (handleMouseWheel(*wheelEvt)) return true;
    }
    
    // Mouse button
    if (auto* btnEvt = std::get_if<event::MouseButtonEvent>(&event)) {
        if (handleMouseButton(*btnEvt)) return true;
    }
    
    // Mouse move
    if (auto* moveEvt = std::get_if<event::MouseMoveEvent>(&event)) {
        if (handleMouseMove(*moveEvt)) return true;
    }
    
    return Widget::onEvent(event);
}

/**
 * @brief Renders the list view.
 * @param renderer The renderer to use for drawing.
 */
void ListView::render(Renderer& renderer) {
    if (!isVisible()) return;
    
    auto rect = getRect();
    auto viewport = getViewportRect();
    
    // Background
    renderer.fillRect(rect, getBackgroundColor());
    
    if (!adapter_ || adapter_->getCount() == 0) {
        // Empty state
        renderScrollbar(renderer);
        return;
    }
    
    // Clip to viewport
    renderer.pushClip(viewport);
    
    // Render selection highlight
    renderSelection(renderer);
    
    // Render visible items
    renderItems(renderer);
    
    renderer.popClip();
    
    // Render scrollbar (on top)
    renderScrollbar(renderer);
}

// ========================================================================
// VIRTUALIZATION CORE
// ========================================================================

/**
 * @brief Calculates the range of items that are currently visible within the viewport.
 * @internal
 */
void ListView::calculateVisibleRange() {
    if (!adapter_) {
        firstVisibleIndex_ = 0;
        lastVisibleIndex_ = 0;
        return;
    }
    
    auto viewport = getViewportRect();
    size_t itemCount = adapter_->getCount();
    
    if (itemCount == 0) {
        firstVisibleIndex_ = 0;
        lastVisibleIndex_ = 0;
        return;
    }
    
    uint32_t itemStride = itemHeight_ + itemSpacing_;
    
    // Calculate visible range (with 1 item buffer on each side)
    size_t firstVisible = static_cast<size_t>(scrollOffset_ / itemStride);
    size_t visibleCount = (viewport.h / itemStride) + 2;  // +2 for partial items
    size_t lastVisible = std::min(firstVisible + visibleCount, itemCount);
    
    // Add buffer (preload items above/below for smooth scrolling)
    if (firstVisible > 0) firstVisible--;
    if (lastVisible < itemCount) lastVisible++;
    
    firstVisibleIndex_ = firstVisible;
    lastVisibleIndex_ = lastVisible;
}

/**
 * @brief Updates the pool of item widgets, binding them to visible data indices.
 * @internal
 */
void ListView::updateWidgetPool() {
    if (!adapter_) return;
    
    size_t itemCount = adapter_->getCount();
    if (itemCount == 0) return;
    
    // Mark all widgets as unbound
    for (auto& pooled : widgetPool_) {
        pooled.boundIndex = size_t(-1);
    }
    
    // Bind widgets to visible range
    for (size_t i = firstVisibleIndex_; i < lastVisibleIndex_; ++i) {
        if (i >= itemCount) break;
        
        // Get or create widget
        IWidget* widget = getPooledWidget(i);
        
        // Update with data from adapter
        adapter_->updateView(i, widget);
        
        // Position widget
        auto viewport = getViewportRect();
        uint32_t itemStride = itemHeight_ + itemSpacing_;
        
        int32_t y = viewport.y + static_cast<int32_t>(i * itemStride - scrollOffset_);
        
        widget->setRect(Rect<int32_t, uint32_t>(
            viewport.x,
            y,
            viewport.w,
            itemHeight_
        ));
        
        widget->setVisible(true);
    }
    
    // Hide unused widgets
    for (auto& pooled : widgetPool_) {
        if (pooled.boundIndex == size_t(-1)) {
            pooled.widget->setVisible(false);
        }
    }
}

/**
 * @brief Retrieves a widget from the pool for a given data index.
 * 
 * If a widget is already bound to the index, it's returned.
 * If not, an unbound widget is reused.
 * If no unbound widgets are available, a new one is created via the adapter.
 * 
 * @param index The data index for which to get a widget.
 * @return A pointer to the widget for the given index.
 * @internal
 */
IWidget* ListView::getPooledWidget(size_t index) {
    // Look for existing binding
    for (auto& pooled : widgetPool_) {
        if (pooled.boundIndex == index) {
            return pooled.widget.get();
        }
    }
    
    // Look for unbound widget
    for (auto& pooled : widgetPool_) {
        if (pooled.boundIndex == size_t(-1)) {
            pooled.boundIndex = index;
            return pooled.widget.get();
        }
    }
    
    // Create new widget
    auto newWidget = adapter_->createView(index);
    addChild(newWidget);
    
    widgetPool_.push_back(PooledWidget{
        .widget = newWidget,
        .boundIndex = index
    });
    
    return newWidget.get();
}

/**
 * @brief Marks a pooled widget as unbound and hides it.
 * @param pooled The pooled widget to return.
 * @internal
 */
void ListView::returnToPool(PooledWidget& pooled) {
    pooled.boundIndex = size_t(-1);
    pooled.widget->setVisible(false);
}

// ========================================================================
// GEOMETRY
// ========================================================================

/**
 * @brief Gets the rectangle for the viewport where items are displayed.
 * This excludes the scrollbar area.
 * @return The viewport rectangle.
 * @internal
 */
Rect<int32_t, uint32_t> ListView::getViewportRect() const {
    auto rect = getRect();
    uint32_t w = rect.w;
    
    if (showScrollbar_ && adapter_ && getTotalContentHeight() > rect.h) {
        w = w > static_cast<uint32_t>(scrollbarWidth_) 
            ? w - static_cast<uint32_t>(scrollbarWidth_) 
            : 0;
    }
    
    return Rect<int32_t, uint32_t>(rect.x, rect.y, w, rect.h);
}

/**
 * @brief Gets the rectangle for the entire scrollbar track.
 * @return The scrollbar rectangle. Returns a zero-sized rect if not shown.
 * @internal
 */
Rect<int32_t, uint32_t> ListView::getScrollbarRect() const {
    auto rect = getRect();
    
    if (!showScrollbar_ || !adapter_ || getTotalContentHeight() <= rect.h) {
        return Rect<int32_t, uint32_t>(0, 0, 0, 0);
    }
    
    auto viewport = getViewportRect();
    int32_t x = rect.x + static_cast<int32_t>(viewport.w);
    
    return Rect<int32_t, uint32_t>(
        x, rect.y, 
        static_cast<uint32_t>(scrollbarWidth_), 
        rect.h
    );
}

/**
 * @brief Gets the rectangle for the scrollbar thumb (the draggable part).
 * @return The scrollbar thumb rectangle.
 * @internal
 */
Rect<int32_t, uint32_t> ListView::getScrollbarThumbRect() const {
    auto scrollbarRect = getScrollbarRect();
    if (scrollbarRect.w == 0) return scrollbarRect;
    
    auto viewport = getViewportRect();
    uint32_t contentHeight = getTotalContentHeight();
    
    float ratio = static_cast<float>(viewport.h) / contentHeight;
    uint32_t thumbHeight = std::max(
        static_cast<uint32_t>(scrollbarRect.h * ratio), 
        20u
    );
    
    float maxScroll = getMaxScrollOffset();
    float scrollRatio = maxScroll > 0.0f ? (scrollOffset_ / maxScroll) : 0.0f;
    
    int32_t thumbY = scrollbarRect.y + static_cast<int32_t>(
        (scrollbarRect.h - thumbHeight) * scrollRatio
    );
    
    return Rect<int32_t, uint32_t>(
        scrollbarRect.x, 
        thumbY, 
        scrollbarRect.w, 
        thumbHeight
    );
}

/**
 * @brief Calculates the total height of all items in the list if they were rendered end-to-end.
 * @return The total content height in pixels.
 * @internal
 */
uint32_t ListView::getTotalContentHeight() const {
    if (!adapter_) return 0;
    
    size_t count = adapter_->getCount();
    if (count == 0) return 0;
    
    uint32_t itemStride = itemHeight_ + itemSpacing_;
    return static_cast<uint32_t>(count * itemStride - itemSpacing_);
}

/**
 * @brief Calculates the maximum possible scroll offset.
 * @return The maximum scroll offset.
 * @internal
 */
float ListView::getMaxScrollOffset() const {
    auto viewport = getViewportRect();
    uint32_t contentHeight = getTotalContentHeight();
    
    if (contentHeight <= viewport.h) return 0.0f;
    
    return static_cast<float>(contentHeight - viewport.h);
}

/**
 * @brief Clamps the current scroll offset to be within valid bounds [0, maxScrollOffset].
 * @internal
 */
void ListView::clampScrollOffset() {
    float maxScroll = getMaxScrollOffset();
    scrollOffset_ = std::clamp(scrollOffset_, 0.0f, maxScroll);
}

// ========================================================================
// EVENT HANDLING
// ========================================================================

/**
 * @brief Handles mouse wheel events to scroll the list.
 * @param evt The mouse wheel event.
 * @return `true` if the event was handled, `false` otherwise.
 * @internal
 */
bool ListView::handleMouseWheel(const event::MouseWheelEvent& evt) {
    auto viewport = getViewportRect();
    
    bool inside = 
        evt.position.x >= viewport.x && 
        evt.position.x < static_cast<int32_t>(viewport.getRight()) &&
        evt.position.y >= viewport.y && 
        evt.position.y < static_cast<int32_t>(viewport.getBottom());
    
    if (!inside) return false;
    
    scrollBy(-static_cast<float>(evt.delta) / 4.0f);
    
    return true;
}

/**
 * @brief Handles mouse button press/release events for item selection and scrollbar interaction.
 * @param evt The mouse button event.
 * @return `true` if the event was handled, `false` otherwise.
 * @internal
 */
bool ListView::handleMouseButton(const event::MouseButtonEvent& evt) {
    if (evt.button != event::MouseButtonEvent::Button::Left) return false;
    
    if (evt.action == event::MouseButtonEvent::Action::Press) {
        // Check scrollbar
        auto thumbRect = getScrollbarThumbRect();
        if (thumbRect.w > 0 &&
            evt.position.x >= thumbRect.x && 
            evt.position.x < static_cast<int32_t>(thumbRect.getRight()) &&
            evt.position.y >= thumbRect.y && 
            evt.position.y < static_cast<int32_t>(thumbRect.getBottom())) {
            
            draggingScrollbar_ = true;
            dragStartPos_ = evt.position;
            dragStartOffset_ = scrollOffset_;
            return true;
        }
        
        // Check track click
        auto scrollbarRect = getScrollbarRect();
        if (scrollbarRect.w > 0 &&
            evt.position.x >= scrollbarRect.x && 
            evt.position.x < static_cast<int32_t>(scrollbarRect.getRight()) &&
            evt.position.y >= scrollbarRect.y && 
            evt.position.y < static_cast<int32_t>(scrollbarRect.getBottom())) {
            
            // Jump scroll
            float ratio = static_cast<float>(evt.position.y - scrollbarRect.y) / scrollbarRect.h;
            scrollOffset_ = ratio * getMaxScrollOffset();
            clampScrollOffset();
            calculateVisibleRange();
            updateWidgetPool();
            invalidate();
            return true;
        }
        
        // Check item click
        auto viewport = getViewportRect();
        if (evt.position.x >= viewport.x && 
            evt.position.x < static_cast<int32_t>(viewport.getRight()) &&
            evt.position.y >= viewport.y && 
            evt.position.y < static_cast<int32_t>(viewport.getBottom())) {
            
            size_t clickedIndex = getItemIndexAtPoint(evt.position);
            if (clickedIndex != size_t(-1)) {
                setSelectedIndex(clickedIndex);
                return true;
            }
        }
        
    } else if (evt.action == event::MouseButtonEvent::Action::Release) {
        if (draggingScrollbar_) {
            draggingScrollbar_ = false;
            return true;
        }
    }
    
    return false;
}

/**
 * @brief Handles mouse move events, primarily for dragging the scrollbar.
 * @param evt The mouse move event.
 * @return `true` if the event was handled, `false` otherwise.
 * @internal
 */
bool ListView::handleMouseMove(const event::MouseMoveEvent& evt) {
    // Handle scrollbar drag
    if (draggingScrollbar_) {
        auto scrollbarRect = getScrollbarRect();
        auto thumbRect = getScrollbarThumbRect();
        
        int32_t deltaY = evt.position.y - dragStartPos_.y;
        float maxScroll = getMaxScrollOffset();
        uint32_t maxThumbOffset = scrollbarRect.h - thumbRect.h;
        
        if (maxThumbOffset > 0) {
            float scrollDelta = (static_cast<float>(deltaY) / maxThumbOffset) * maxScroll;
            scrollOffset_ = dragStartOffset_ + scrollDelta;
            clampScrollOffset();
            calculateVisibleRange();
            updateWidgetPool();
            invalidate();
        }
        
        return true;
    }
    
    // Update hover state
    bool wasHovering = hoveringScrollbar_;
    hoveringScrollbar_ = isPointInScrollbar(evt.position);
    
    if (wasHovering != hoveringScrollbar_) {
        invalidate();
    }
    
    return false;
}

/**
 * @brief Checks if a point is within the scrollbar thumb.
 * @param point The point to check.
 * @return `true` if the point is inside the thumb, `false` otherwise.
 * @internal
 */
bool ListView::isPointInScrollbar(const Point<int32_t>& point) const {
    auto thumbRect = getScrollbarThumbRect();
    return thumbRect.w > 0 && 
           point.x >= thumbRect.x && 
           point.x < static_cast<int32_t>(thumbRect.getRight()) &&
           point.y >= thumbRect.y && 
           point.y < static_cast<int32_t>(thumbRect.getBottom());
}

/**
 * @brief Gets the data index of the item at a specific point.
 * @param point The point to check, in widget-relative coordinates.
 * @return The index of the item, or `size_t(-1)` if no item is at that point.
 * @internal
 */
size_t ListView::getItemIndexAtPoint(const Point<int32_t>& point) const {
    if (!adapter_) return size_t(-1);
    
    auto viewport = getViewportRect();
    
    // Translate to content space
    int32_t relY = point.y - viewport.y + static_cast<int32_t>(scrollOffset_);
    
    if (relY < 0) return size_t(-1);
    
    uint32_t itemStride = itemHeight_ + itemSpacing_;
    size_t index = static_cast<size_t>(relY) / itemStride;
    
    if (index >= adapter_->getCount()) return size_t(-1);
    
    return index;
}

// ========================================================================
// RENDERING
// ========================================================================

/**
 * @brief Renders the visible items.
 * 
 * This method simply calls the base `Widget::render` method, which will in turn
 * render all child widgets (our pooled item views). The clipping rect set in
 * the main `render` method ensures only visible portions are drawn.
 * 
 * @param renderer The renderer to use for drawing.
 * @internal
 */
void ListView::renderItems(Renderer& renderer) {
    // Items are already rendered as children by Widget::render()
    // This just renders them with proper clipping
    Widget::render(renderer);
}

/**
 * @brief Renders the scrollbar track and thumb.
 * @param renderer The renderer to use for drawing.
 * @internal
 */
void ListView::renderScrollbar(Renderer& renderer) {
    if (!showScrollbar_) return;
    
    auto scrollbarRect = getScrollbarRect();
    if (scrollbarRect.w == 0) return;
    
    auto thumbRect = getScrollbarThumbRect();
    
    // Track
    renderer.fillRect(scrollbarRect, Color(240, 240, 240, 200));
    
    // Thumb
    Color thumbColor = hoveringScrollbar_ ? scrollbarHoverColor_ : scrollbarColor_;
    renderer.fillRect(thumbRect, thumbColor);
}

/**
 * @brief Renders the highlight for the selected item.
 * @param renderer The renderer to use for drawing.
 * @internal
 */
void ListView::renderSelection(Renderer& renderer) {
    if (selectedIndex_ == size_t(-1)) return;
    if (!adapter_ || selectedIndex_ >= adapter_->getCount()) return;
    
    // Check if selected item is visible
    if (selectedIndex_ < firstVisibleIndex_ || selectedIndex_ >= lastVisibleIndex_) {
        return;
    }
    
    auto viewport = getViewportRect();
    uint32_t itemStride = itemHeight_ + itemSpacing_;
    
    int32_t y = viewport.y + static_cast<int32_t>(
        selectedIndex_ * itemStride - scrollOffset_
    );
    
    Rect<int32_t, uint32_t> selectionRect(
        viewport.x, y, 
        viewport.w, itemHeight_
    );
    
    renderer.fillRect(selectionRect, selectionColor_);
}

} // namespace frqs::widget
