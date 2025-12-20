// include/widget/combobox.hpp
#pragma once

#include "iwidget.hpp"
#include "button.hpp"
#include "list_view.hpp"
#include "list_adapter.hpp"
#include <memory>
#include <functional>

namespace frqs::widget {

// ============================================================================
// COMBO BOX (Dropdown Selector)
// ============================================================================

/**
 * @brief Dropdown selector widget with virtual list support.
 * 
 * Components:
 * - Header Button: Shows selected item, click to toggle dropdown
 * - Dropdown List: Virtual ListView that appears below header
 * 
 * Overlay Support:
 * The dropdown list renders in an "overlay" mode, which bypasses parent
 * clipping. This allows the dropdown to extend beyond the ComboBox's
 * container bounds (essential for small containers).
 * 
 * Usage Example:
 * ```cpp
 * // Create adapter
 * class StringAdapter : public IListAdapter {
 *     std::vector<std::string> items_;
 * public:
 *     size_t getCount() const override { return items_.size(); }
 *     
 *     std::shared_ptr<IWidget> createView(size_t) override {
 *         return std::make_shared<Label>();
 *     }
 *     
 *     void updateView(size_t idx, IWidget* view) override {
 *         auto* label = static_cast<Label*>(view);
 *         label->setText(items_[idx]);
 *     }
 * };
 * 
 * // Create ComboBox
 * auto combo = std::make_shared<ComboBox>();
 * combo->setAdapter(adapter);
 * combo->setSelectedIndex(0);
 * combo->setOnSelectionChanged([](size_t idx) {
 *     std::println("Selected: {}", idx);
 * });
 * ```
 */
class ComboBox : public Widget {
public:
    using SelectionCallback = std::function<void(size_t)>;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;

    // Components
    std::shared_ptr<Button> headerButton_;
    std::shared_ptr<ListView> dropdownList_;

    // State
    bool isOpen_ = false;
    size_t selectedIndex_ = size_t(-1);

    // Configuration
    uint32_t dropdownMaxHeight_ = 200;
    uint32_t itemHeight_ = 35;

    // Styling
    Color headerColor_ = Color(250, 250, 250);
    Color headerHoverColor_ = Color(240, 240, 240);
    Color dropdownBgColor_ = colors::White;
    Color dropdownBorderColor_ = Color(189, 195, 199);
    float borderWidth_ = 1.0f;

    // Callback
    SelectionCallback onSelectionChanged_;

public:
    explicit ComboBox();
    ~ComboBox() override;

    // ========================================================================
    // ADAPTER MANAGEMENT
    // ========================================================================

    /**
     * @brief Set the data adapter.
     */
    void setAdapter(std::shared_ptr<IListAdapter> adapter);

    /**
     * @brief Get current adapter.
     */
    std::shared_ptr<IListAdapter> getAdapter() const;

    /**
     * @brief Notify that data has changed.
     */
    void notifyDataChanged();

    // ========================================================================
    // SELECTION
    // ========================================================================

    void setSelectedIndex(size_t index);
    size_t getSelectedIndex() const noexcept { return selectedIndex_; }
    void clearSelection() noexcept { selectedIndex_ = size_t(-1); }

    void setOnSelectionChanged(SelectionCallback callback) {
        onSelectionChanged_ = std::move(callback);
    }

    // ========================================================================
    // DROPDOWN CONTROL
    // ========================================================================

    void openDropdown();
    void closeDropdown();
    void toggleDropdown();

    bool isDropdownOpen() const noexcept { return isOpen_; }

    // ========================================================================
    // CONFIGURATION
    // ========================================================================

    void setDropdownMaxHeight(uint32_t height) noexcept { dropdownMaxHeight_ = height; }
    uint32_t getDropdownMaxHeight() const noexcept { return dropdownMaxHeight_; }

    void setItemHeight(uint32_t height) noexcept;
    uint32_t getItemHeight() const noexcept { return itemHeight_; }

    // Styling
    void setHeaderColor(const Color& color) noexcept { headerColor_ = color; }
    void setDropdownBackgroundColor(const Color& color) noexcept;
    void setDropdownBorderColor(const Color& color) noexcept { dropdownBorderColor_ = color; }

    // ========================================================================
    // WIDGET OVERRIDES
    // ========================================================================

    void setRect(const Rect<int32_t, uint32_t>& rect) override;
    bool onEvent(const event::Event& event) override;
    void render(Renderer& renderer) override;

private:
    // ========================================================================
    // INTERNAL
    // ========================================================================

    void updateHeaderText();
    void positionDropdown();
    void handleHeaderClick();
    void handleListSelection(size_t index);

    // Check if click is outside ComboBox (for auto-close)
    bool isPointOutside(const Point<int32_t>& point) const;
};

} // namespace frqs::widget