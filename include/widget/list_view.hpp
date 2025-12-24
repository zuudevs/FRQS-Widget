// include/widget/list_view.hpp
#pragma once

#include "iwidget.hpp"
#include "list_adapter.hpp"
#include <memory>
#include <vector>
#include <unordered_map>

namespace frqs::widget {

// src/widget/list_view.cpp (Conceptual - already partially implemented)
struct PooledWidget {
    std::shared_ptr<IWidget> widget;
    size_t boundIndex = size_t(-1);  // -1 = available for reuse
};

std::vector<PooledWidget> widgetPool_;

IWidget* getPooledWidget(size_t index) {
    // 1. Check for existing binding (already in use)
    for (auto& p : widgetPool_) {
        if (p.boundIndex == index) return p.widget.get();
    }
    
    // 2. ✅ CRITICAL: Reuse unbound widget (TRUE RECYCLING)
    for (auto& p : widgetPool_) {
        if (p.boundIndex == size_t(-1)) {
            p.boundIndex = index;  // Claim it
            return p.widget.get();
        }
    }
    
    // 3. Only create if pool exhausted
    auto newWidget = adapter_->createView(index);
    widgetPool_.push_back({newWidget, index});
    addChild(newWidget);  // Add to widget tree
    return newWidget.get();
}

void updateWidgetPool() {
    // Mark all as unbound
    for (auto& p : widgetPool_) {
        p.boundIndex = size_t(-1);
    }
    
    // Rebind visible widgets
    for (size_t i = firstVisibleIndex_; i < lastVisibleIndex_; ++i) {
        IWidget* widget = getPooledWidget(i);  // Recycles from pool!
        adapter_->updateView(i, widget);
        widget->setVisible(true);
        // ... position widget ...
    }
    
    // Hide unused (but keep in pool for next frame!)
    for (auto& p : widgetPool_) {
        if (p.boundIndex == size_t(-1)) {
            p.widget->setVisible(false);
        }
    }
}

// ============================================================================
// LIST VIEW (Virtual Scrolling List)
// ============================================================================

/**
 * @brief High-performance virtual scrolling list widget.
 * 
 * Features:
 * - UI Virtualization: Only renders visible items
 * - Widget Recycling: Reuses widget instances as they scroll
 * - Scalability: Can handle millions of items efficiently
 * - Fixed Height: V1 assumes all items have the same height
 * 
 * Performance:
 * - Memory: O(visible items) instead of O(total items)
 * - Rendering: O(visible items) instead of O(total items)
 * - Scrolling: 60 FPS even with millions of items
 * 
 * Usage Example:
 * ```cpp
 * // Create adapter
 * class MyAdapter : public IListAdapter {
 *     std::vector<std::string> data_;
 * public:
 *     size_t getCount() const override { return data_.size(); }
 *     
 *     std::shared_ptr<IWidget> createView(size_t) override {
 *         return std::make_shared<Label>();
 *     }
 *     
 *     void updateView(size_t idx, IWidget* view) override {
 *         auto* label = static_cast<Label*>(view);
 *         label->setText(data_[idx]);
 *     }
 * };
 * 
 * // Use ListView
 * auto adapter = std::make_shared<MyAdapter>(myData);
 * auto listView = std::make_shared<ListView>();
 * listView->setAdapter(adapter);
 * ```
 */
class ListView : public Widget {
public:
    using SelectionCallback = std::function<void(size_t)>;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;

    // Adapter (data source)
    std::shared_ptr<IListAdapter> adapter_;

    // Virtualization state
    size_t firstVisibleIndex_ = 0;
    size_t lastVisibleIndex_ = 0;
    float scrollOffset_ = 0.0f;

    // Widget pool (recycler)
    struct PooledWidget {
        std::shared_ptr<IWidget> widget;
        size_t boundIndex = size_t(-1);  // -1 = not bound
    };
    std::vector<PooledWidget> widgetPool_;

    // Configuration
    uint32_t itemHeight_ = 40;
    uint32_t itemSpacing_ = 0;
    bool showScrollbar_ = true;
    float scrollbarWidth_ = 12.0f;

    // Selection
    size_t selectedIndex_ = size_t(-1);
    Color selectionColor_ = Color(52, 152, 219, 100);
    SelectionCallback onSelectionChanged_;

    // Scrollbar state
    bool draggingScrollbar_ = false;
    bool hoveringScrollbar_ = false;
    Point<int32_t> dragStartPos_;
    float dragStartOffset_ = 0.0f;

    // Visual
    Color scrollbarColor_ = Color(150, 150, 150, 180);
    Color scrollbarHoverColor_ = Color(120, 120, 120, 220);

public:
    explicit ListView();
    ~ListView() override;

    // ========================================================================
    // ADAPTER MANAGEMENT
    // ========================================================================

    /**
     * @brief Set the data adapter.
     * @param adapter Data source (nullptr to clear)
     * 
     * Replaces the current adapter and rebuilds the widget pool.
     */
    void setAdapter(std::shared_ptr<IListAdapter> adapter);

    /**
     * @brief Get current adapter.
     */
    std::shared_ptr<IListAdapter> getAdapter() const noexcept { return adapter_; }

    /**
     * @brief Notify that data has changed (triggers refresh).
     * 
     * Call this when your data source changes (items added/removed/modified).
     * The list will recalculate bounds and update visible items.
     */
    void notifyDataChanged();

    // ========================================================================
    // CONFIGURATION
    // ========================================================================

    void setItemHeight(uint32_t height) noexcept;
    uint32_t getItemHeight() const noexcept { return itemHeight_; }

    void setItemSpacing(uint32_t spacing) noexcept;
    uint32_t getItemSpacing() const noexcept { return itemSpacing_; }

    void setShowScrollbar(bool show) noexcept { showScrollbar_ = show; }
    bool isShowingScrollbar() const noexcept { return showScrollbar_; }

    void setScrollbarWidth(float width) noexcept { scrollbarWidth_ = width; }

    // ========================================================================
    // SELECTION
    // ========================================================================

    void setSelectedIndex(size_t index);
    size_t getSelectedIndex() const noexcept { return selectedIndex_; }
    void clearSelection() noexcept { selectedIndex_ = size_t(-1); }

    void setSelectionColor(const Color& color) noexcept { selectionColor_ = color; }

    void setOnSelectionChanged(SelectionCallback callback) {
        onSelectionChanged_ = std::move(callback);
    }

    // ========================================================================
    // SCROLLING
    // ========================================================================

    void scrollTo(size_t index);
    void scrollToTop() { scrollTo(0); }
    void scrollToBottom();

    void scrollBy(float delta);

    float getScrollOffset() const noexcept { return scrollOffset_; }

    // ========================================================================
    // WIDGET OVERRIDES
    // ========================================================================

    void setRect(const Rect<int32_t, uint32_t>& rect) override;
    bool onEvent(const event::Event& event) override;
    void render(Renderer& renderer) override;

private:
    // ========================================================================
    // VIRTUALIZATION CORE
    // ========================================================================

    // Calculate which items are visible
    void calculateVisibleRange();

    // Update widget pool to match visible range
    void updateWidgetPool();

    // Get or create a widget from the pool
    IWidget* getPooledWidget(size_t index);

    // Return a widget to the pool (unbind)
    void returnToPool(PooledWidget& pooled);

    // ========================================================================
    // GEOMETRY
    // ========================================================================

    Rect<int32_t, uint32_t> getViewportRect() const;
    Rect<int32_t, uint32_t> getScrollbarRect() const;
    Rect<int32_t, uint32_t> getScrollbarThumbRect() const;

    uint32_t getTotalContentHeight() const;
    float getMaxScrollOffset() const;
    void clampScrollOffset();

    // ========================================================================
    // EVENT HANDLING
    // ========================================================================

    bool handleMouseWheel(const event::MouseWheelEvent& evt);
    bool handleMouseButton(const event::MouseButtonEvent& evt);
    bool handleMouseMove(const event::MouseMoveEvent& evt);

    bool isPointInScrollbar(const Point<int32_t>& point) const;
    size_t getItemIndexAtPoint(const Point<int32_t>& point) const;

    // ========================================================================
    // RENDERING
    // ========================================================================

    void renderItems(Renderer& renderer);
    void renderScrollbar(Renderer& renderer);
    void renderSelection(Renderer& renderer);
};

} // namespace frqs::widget