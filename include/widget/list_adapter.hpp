// include/widget/list_adapter.hpp
#pragma once

#include "iwidget.hpp"
#include <memory>
#include <cstddef>
#include <vector>

namespace frqs::widget {

// ============================================================================
// LIST ADAPTER INTERFACE (Model-View Separation)
// ============================================================================

/**
 * @brief Adapter interface for providing data to virtualized lists.
 * 
 * This interface decouples the data source from the view rendering.
 * Implementations can wrap std::vector, databases, or any data source.
 * 
 * Thread Safety: Implementations should be thread-safe if data can be
 * modified from background threads.
 * 
 * Performance: getCount() and updateView() are called frequently during
 * scrolling - keep them fast (< 1ms ideally).
 */
class IListAdapter {
public:
    virtual ~IListAdapter() noexcept = default;

    // ========================================================================
    // DATA ACCESS
    // ========================================================================

    /**
     * @brief Get total number of items in the dataset.
     * @return Total count (can be millions)
     * 
     * Called frequently - must be O(1).
     */
    virtual size_t getCount() const = 0;

    /**
     * @brief Create a new view widget for displaying an item.
     * @param index Item index (for initialization if needed)
     * @return Newly created widget instance
     * 
     * Called only when the widget pool needs to grow.
     * The returned widget will be reused for different indices.
     * 
     * Performance: This is called rarely (only when expanding pool),
     * so it's okay to allocate here.
     */
    virtual std::shared_ptr<IWidget> createView(size_t index) = 0;

    /**
     * @brief Update an existing view widget with data from a specific index.
     * @param index Item index to display
     * @param view Widget to update (guaranteed to be from createView())
     * 
     * Called every frame during scrolling - MUST BE FAST.
     * Typical implementation: cast view to your concrete type, update text/colors.
     * 
     * Example:
     * ```cpp
     * void updateView(size_t index, IWidget* view) override {
     *     auto* label = dynamic_cast<Label*>(view);
     *     label->setText(myData[index]);
     * }
     * ```
     */
    virtual void updateView(size_t index, IWidget* view) = 0;

    // ========================================================================
    // OPTIONAL: CUSTOM HEIGHT (Future expansion)
    // ========================================================================

    /**
     * @brief Get height for a specific item (optional, V1 uses fixed height).
     * @param index Item index
     * @return Height in pixels
     * 
     * Override this for variable-height items (Phase 5).
     * Default implementation returns a fixed height.
     */
    virtual uint32_t getItemHeight([[maybe_unused]] size_t index) const {
        return 40;  // Default: 40px per item
    }
};

// ============================================================================
// SIMPLE ADAPTER (Convenience Base Class)
// ============================================================================

/**
 * @brief Simple adapter for small datasets (< 1000 items).
 * 
 * Creates one widget per item (no virtualization benefit).
 * Use this for prototyping or when you have very few items.
 */
template <typename DataType>
class SimpleListAdapter : public IListAdapter {
protected:
    std::vector<DataType> data_;
    uint32_t itemHeight_ = 40;

public:
    explicit SimpleListAdapter(std::vector<DataType> data, uint32_t itemHeight = 40)
        : data_(std::move(data)), itemHeight_(itemHeight) {}

    size_t getCount() const override {
        return data_.size();
    }

    uint32_t getItemHeight([[maybe_unused]] size_t index) const override {
        return itemHeight_;
    }

    // Derived class must implement:
    // - createView()
    // - updateView()
};

} // namespace frqs::widget