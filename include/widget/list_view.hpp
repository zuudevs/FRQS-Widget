// include/widget/list_view.hpp
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
 * @brief High-performance virtual scrolling list widget.
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

    void setAdapter(std::shared_ptr<IListAdapter> adapter);
    std::shared_ptr<IListAdapter> getAdapter() const noexcept { return adapter_; }
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
    void calculateVisibleRange();
    void updateWidgetPool();
    IWidget* getPooledWidget(size_t index);
    void returnToPool(PooledWidget& pooled);

    Rect<int32_t, uint32_t> getViewportRect() const;
    Rect<int32_t, uint32_t> getScrollbarRect() const;
    Rect<int32_t, uint32_t> getScrollbarThumbRect() const;

    uint32_t getTotalContentHeight() const;
    float getMaxScrollOffset() const;
    void clampScrollOffset();

    bool handleMouseWheel(const event::MouseWheelEvent& evt);
    bool handleMouseButton(const event::MouseButtonEvent& evt);
    bool handleMouseMove(const event::MouseMoveEvent& evt);

    bool isPointInScrollbar(const Point<int32_t>& point) const;
    size_t getItemIndexAtPoint(const Point<int32_t>& point) const;

    void renderItems(Renderer& renderer);
    void renderScrollbar(Renderer& renderer);
    void renderSelection(Renderer& renderer);
};

} // namespace frqs::widget