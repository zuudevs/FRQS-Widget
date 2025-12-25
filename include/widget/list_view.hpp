/**
 * @file list_view.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines the ListView widget, a high-performance, data-driven list with virtual scrolling.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "iwidget.hpp"
#include "list_adapter.hpp"
#include <memory>
#include <vector>
#include <functional>

namespace frqs::widget {

// ============================================================================
// LIST VIEW (Virtual Scrolling List)
// ============================================================================

/**
 * @brief A high-performance list widget that uses virtual scrolling to display large data sets efficiently.
 * @details The ListView only creates and renders widgets for the items that are currently visible on screen.
 * As the user scrolls, it recycles these widgets and re-binds them to new data from an `IListAdapter`.
 * This approach maintains high performance and low memory usage regardless of the total number of items in the list.
 */
class ListView : public Widget {
public:
    /**
     * @brief A callback function type that is invoked when the selected item changes.
     * @param index The index of the newly selected item, or size_t(-1) if the selection is cleared.
     */
    using SelectionCallback = std::function<void(size_t)>;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_; ///< PIMPL pointer for private implementation details.

    // Adapter (data source)
    std::shared_ptr<IListAdapter> adapter_; ///< The data provider for the list view.

    // Virtualization state
    size_t firstVisibleIndex_ = 0;   ///< The index of the first item currently visible in the viewport.
    size_t lastVisibleIndex_ = 0;    ///< The index of the last item currently visible in the viewport.
    float scrollOffset_ = 0.0f;      ///< The current vertical scroll position in pixels.

    // Widget pool (recycler)
    /**
     * @brief Represents a widget held in the recycling pool.
     */
    struct PooledWidget {
        std::shared_ptr<IWidget> widget; ///< The actual widget.
        size_t boundIndex = size_t(-1);  ///< The data index this widget is currently bound to. `size_t(-1)` means it's available.
    };
    std::vector<PooledWidget> widgetPool_; ///< A pool of recycled widgets to be reused for visible items.

    // Configuration
    uint32_t itemHeight_ = 40;       ///< The fixed height of each item in the list, in pixels.
    uint32_t itemSpacing_ = 0;       ///< The vertical space between items, in pixels.
    bool showScrollbar_ = true;      ///< Flag to control the visibility of the scrollbar.
    float scrollbarWidth_ = 12.0f;   ///< The width of the scrollbar, in pixels.

    // Selection
    size_t selectedIndex_ = size_t(-1);                   ///< The index of the currently selected item. `size_t(-1)` for no selection.
    Color selectionColor_ = Color(52, 152, 219, 100);   ///< The background color of the selected item.
    SelectionCallback onSelectionChanged_;              ///< Callback function triggered on selection change.

    // Scrollbar state
    bool draggingScrollbar_ = false;  ///< True if the user is currently dragging the scrollbar thumb.
    bool hoveringScrollbar_ = false;  ///< True if the mouse is currently over the scrollbar.
    Point<int32_t> dragStartPos_;     ///< The mouse position where the scrollbar drag started.
    float dragStartOffset_ = 0.0f;    ///< The scroll offset at the moment the drag started.

    // Visual
    Color scrollbarColor_ = Color(150, 150, 150, 180);       ///< The color of the scrollbar thumb.
    Color scrollbarHoverColor_ = Color(120, 120, 120, 220);  ///< The color of the scrollbar thumb when hovered.

public:
    /**
     * @brief Constructs a new ListView object.
     */
    explicit ListView();
    /**
     * @brief Destroys the ListView object.
     */
    ~ListView() override;

    // ========================================================================
    // ADAPTER MANAGEMENT
    // ========================================================================

    /**
     * @brief Sets the data adapter for the list view.
     * @details The adapter is responsible for providing the data and creating the widgets for each item.
     * @param[in] adapter The `IListAdapter` to use as the data source.
     */
    void setAdapter(std::shared_ptr<IListAdapter> adapter);

    /**
     * @brief Gets the current data adapter.
     * @return A shared pointer to the current `IListAdapter`.
     */
    std::shared_ptr<IListAdapter> getAdapter() const noexcept { return adapter_; }

    /**
     * @brief Notifies the list view that the underlying data in the adapter has changed.
     * @details This should be called whenever the data set is modified (items added, removed, or changed)
     * to force the list view to refresh its content.
     */
    void notifyDataChanged();

    // ========================================================================
    // CONFIGURATION
    // ========================================================================

    /**
     * @brief Sets the fixed height for each item in the list.
     * @param[in] height The height in pixels.
     */
    void setItemHeight(uint32_t height) noexcept;

    /**
     * @brief Gets the fixed height of each item.
     * @return The item height in pixels.
     */
    uint32_t getItemHeight() const noexcept { return itemHeight_; }

    /**
     * @brief Sets the vertical spacing between items.
     * @param[in] spacing The spacing in pixels.
     */
    void setItemSpacing(uint32_t spacing) noexcept;

    /**
     * @brief Gets the vertical spacing between items.
     * @return The item spacing in pixels.
     */
    uint32_t getItemSpacing() const noexcept { return itemSpacing_; }

    /**
     * @brief Sets whether the scrollbar should be displayed.
     * @param[in] show True to show the scrollbar, false to hide it.
     */
    void setShowScrollbar(bool show) noexcept { showScrollbar_ = show; }

    /**
     * @brief Checks if the scrollbar is currently being shown.
     * @return True if the scrollbar is shown, false otherwise.
     */
    bool isShowingScrollbar() const noexcept { return showScrollbar_; }

    /**
     * @brief Sets the width of the scrollbar.
     * @param[in] width The width in pixels.
     */
    void setScrollbarWidth(float width) noexcept { scrollbarWidth_ = width; }

    // ========================================================================
    // SELECTION
    // ========================================================================

    /**
     * @brief Programmatically sets the selected item.
     * @param[in] index The index of the item to select.
     */
    void setSelectedIndex(size_t index);

    /**
     * @brief Gets the index of the currently selected item.
     * @return The selected index, or `size_t(-1)` if no item is selected.
     */
    size_t getSelectedIndex() const noexcept { return selectedIndex_; }

    /**
     * @brief Clears the current selection.
     */
    void clearSelection() noexcept { selectedIndex_ = size_t(-1); }

    /**
     * @brief Sets the background color for the selected item.
     * @param[in] color The color to use for highlighting the selection.
     */
    void setSelectionColor(const Color& color) noexcept { selectionColor_ = color; }

    /**
     * @brief Sets the callback function to be invoked when the selection changes.
     * @param[in] callback The function to call.
     */
    void setOnSelectionChanged(SelectionCallback callback) {
        onSelectionChanged_ = std::move(callback);
    }

    // ========================================================================
    // SCROLLING
    // ========================================================================

    /**
     * @brief Scrolls the list to make the item at the specified index visible.
     * @param[in] index The index of the item to scroll to.
     */
    void scrollTo(size_t index);

    /**
     * @brief Scrolls the list to the very top.
     */
    void scrollToTop() { scrollTo(0); }

    /**
     * @brief Scrolls the list to the very bottom.
     */
    void scrollToBottom();

    /**
     * @brief Scrolls the list by a given amount.
     * @param[in] delta The number of pixels to scroll. A positive value scrolls down, a negative value scrolls up.
     */
    void scrollBy(float delta);

    /**
     * @brief Gets the current vertical scroll offset.
     * @return The scroll offset in pixels.
     */
    float getScrollOffset() const noexcept { return scrollOffset_; }

    // ========================================================================
    // WIDGET OVERRIDES
    // ========================================================================

    void setRect(const Rect<int32_t, uint32_t>& rect) override;
    bool onEvent(const event::Event& event) override;
    void render(Renderer& renderer) override;

private:
    /**
     * @brief Calculates the range of visible item indices based on the current scroll offset.
     */
    void calculateVisibleRange();

    /**
     * @brief Updates the widget pool, recycling and re-binding widgets for the current visible range.
     */
    void updateWidgetPool();

    /**
     * @brief Retrieves a widget from the pool for a specific data index.
     * @param[in] index The data index for which to get a widget.
     * @return A pointer to the configured widget.
     */
    IWidget* getPooledWidget(size_t index);

    /**
     * @brief Returns a widget to the pool, marking it as available for recycling.
     * @param[in,out] pooled The pooled widget to return.
     */
    void returnToPool(PooledWidget& pooled);

    /**
     * @brief Gets the rectangle of the list's content area, excluding the scrollbar.
     * @return The viewport rectangle.
     */
    Rect<int32_t, uint32_t> getViewportRect() const;

    /**
     * @brief Gets the rectangle for the entire scrollbar track.
     * @return The scrollbar track rectangle.
     */
    Rect<int32_t, uint32_t> getScrollbarRect() const;

    /**
     * @brief Gets the rectangle for the scrollbar's draggable thumb.
     * @return The scrollbar thumb rectangle.
     */
    Rect<int32_t, uint32_t> getScrollbarThumbRect() const;

    /**
     * @brief Calculates the total height of all items in the list if they were rendered.
     * @return The total content height in pixels.
     */
    uint32_t getTotalContentHeight() const;

    /**
     * @brief Calculates the maximum possible scroll offset.
     * @return The maximum scroll offset in pixels.
     */
    float getMaxScrollOffset() const;

    /**
     * @brief Clamps the current scroll offset to be within valid bounds [0, max_scroll_offset].
     */
    void clampScrollOffset();

    /**
     * @brief Handles mouse wheel events for scrolling.
     * @param[in] evt The mouse wheel event.
     * @return True if the event was handled, false otherwise.
     */
    bool handleMouseWheel(const event::MouseWheelEvent& evt);
    
    /**
     * @brief Handles mouse button press/release events for selection and scrollbar dragging.
     * @param[in] evt The mouse button event.
     * @return True if the event was handled, false otherwise.
     */
    bool handleMouseButton(const event::MouseButtonEvent& evt);
    
    /**
     * @brief Handles mouse move events for scrollbar dragging and hover effects.
     * @param[in] evt The mouse move event.
     * @return True if the event was handled, false otherwise.
     */
    bool handleMouseMove(const event::MouseMoveEvent& evt);

    /**
     * @brief Checks if a point is within the scrollbar's track area.
     * @param[in] point The point to check.
     * @return True if the point is inside the scrollbar, false otherwise.
     */
    bool isPointInScrollbar(const Point<int32_t>& point) const;

    /**
     * @brief Determines the item index at a given point within the list view.
     * @param[in] point The point to check.
     * @return The index of the item at the point, or `size_t(-1)` if no item is there.
     */
    size_t getItemIndexAtPoint(const Point<int32_t>& point) const;

    /**
     * @brief Renders the visible items.
     * @param[in,out] renderer The renderer to draw with.
     */
    void renderItems(Renderer& renderer);

    /**
     * @brief Renders the scrollbar.
     * @param[in,out] renderer The renderer to draw with.
     */
    void renderScrollbar(Renderer& renderer);
    
    /**
     * @brief Renders the highlight for the selected item.
     * @param[in,out] renderer The renderer to draw with.
     */
    void renderSelection(Renderer& renderer);
};

} // namespace frqs::widget