/**
 * @file combobox.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines the ComboBox (dropdown selector) widget.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */
 
#pragma once

#include "iwidget.hpp"
#include "button.hpp"
#include "list_view.hpp"
#include "list_adapter.hpp"
#include <memory>
#include <functional>

namespace frqs::widget {

/**
 * @class ComboBox
 * @brief A dropdown selector widget that displays a list of items when clicked.
 * 
 * This widget is composed of two main parts:
 * - A header button that displays the currently selected item. Clicking this
 *   button toggles the visibility of the dropdown list.
 * - A virtual `ListView` that appears as a dropdown, showing the available items.
 * 
 * @section overlay_support Overlay Rendering
 * The dropdown list renders in an "overlay" mode, which bypasses parent
 * clipping. This is crucial as it allows the dropdown to extend beyond the
 * bounds of its parent container, which is essential for ComboBoxes placed
 * within small areas.
 * 
 * @section usage_example Usage Example
 * @code
 * // 1. Define a custom data adapter by inheriting from IListAdapter
 * class MyStringAdapter : public IListAdapter {
 *     std::vector<std::string> items_;
 * public:
 *     // Populate items...
 *     size_t getCount() const override { return items_.size(); }
 *     
 *     // Create a new view for an item (e.g., a Label)
 *     std::shared_ptr<IWidget> createView(size_t) override {
 *         return std::make_shared<Label>();
 *     }
 *     
 *     // Update an existing view with data for a specific index
 *     void updateView(size_t idx, IWidget* view) override {
 *         auto* label = static_cast<Label*>(view);
 *         label->setText(items_[idx]);
 *     }
 * };
 * 
 * // 2. Create and configure the ComboBox
 * auto myAdapter = std::make_shared<MyStringAdapter>();
 * auto combo = std::make_shared<ComboBox>();
 * combo->setAdapter(myAdapter);
 * combo->setSelectedIndex(0);
 * 
 * // 3. Set a callback for when the selection changes
 * combo->setOnSelectionChanged([](size_t idx) {
 *     std::println("Selected item at index: {}", idx);
 * });
 * @endcode
 */
class ComboBox : public Widget {
public:
    /**
     * @brief Callback function type for when the selected item changes.
     * @param index The index of the newly selected item.
     */
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
    /**
     * @brief Constructs a new ComboBox widget.
     */
    explicit ComboBox();

    /**
     * @brief Destructor.
     */
    ~ComboBox() override;

    /**
     * @brief Sets the data adapter that provides the items for the dropdown list.
     * @param adapter A shared pointer to an object implementing the `IListAdapter` interface.
     */
    void setAdapter(std::shared_ptr<IListAdapter> adapter);

    /**
     * @brief Gets the current data adapter.
     * @return A shared pointer to the current `IListAdapter`.
     */
    std::shared_ptr<IListAdapter> getAdapter() const;

    /**
     * @brief Refreshes the ComboBox display after the adapter's data has changed.
     * 
     * Call this method if you modify the data source of the adapter
     * (e.g., add or remove items) to ensure the ComboBox reflects the changes.
     */
    void notifyDataChanged();

    /**
     * @brief Sets the currently selected item by its index.
     * @param index The index of the item to select.
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
     * @brief Sets the callback function to be invoked when the selection changes.
     * @param callback The function to call.
     */
    void setOnSelectionChanged(SelectionCallback callback) {
        onSelectionChanged_ = std::move(callback);
    }

    /**
     * @brief Opens the dropdown list to show the available items.
     */
    void openDropdown();

    /**
     * @brief Closes the dropdown list.
     */
    void closeDropdown();

    /**
     * @brief Toggles the visibility of the dropdown list.
     */
    void toggleDropdown();

    /**
     * @brief Checks if the dropdown list is currently open.
     * @return True if the dropdown is open, false otherwise.
     */
    bool isDropdownOpen() const noexcept { return isOpen_; }

    /**
     * @brief Sets the maximum height of the dropdown list.
     * @param height The maximum height in pixels.
     */
    void setDropdownMaxHeight(uint32_t height) noexcept { dropdownMaxHeight_ = height; }
    
    /**
     * @brief Gets the maximum height of the dropdown list.
     * @return The maximum height in pixels.
     */
    uint32_t getDropdownMaxHeight() const noexcept { return dropdownMaxHeight_; }

    /**
     * @brief Sets the height for each item in the dropdown list.
     * @param height The item height in pixels.
     */
    void setItemHeight(uint32_t height) noexcept;
    
    /**
     * @brief Gets the height of each item in the dropdown list.
     * @return The item height in pixels.
     */
    uint32_t getItemHeight() const noexcept { return itemHeight_; }

    /**
     * @brief Sets the background color of the header button.
     * @param color The new color.
     */
    void setHeaderColor(const Color& color) noexcept { headerColor_ = color; }
    
    /**
     * @brief Sets the background color of the dropdown list.
     * @param color The new color.
     */
    void setDropdownBackgroundColor(const Color& color) noexcept;
    
    /**
     * @brief Sets the border color of the dropdown list.
     * @param color The new color.
     */
    void setDropdownBorderColor(const Color& color) noexcept { dropdownBorderColor_ = color; }

    /**
     * @brief Sets the rectangle (position and size) of the widget.
     * @param rect The new rectangle.
     */
    void setRect(const Rect<int32_t, uint32_t>& rect) override;
    
    /**
     * @brief Handles incoming events for the widget.
     * @param event The event to process.
     * @return True if the event was handled, false otherwise.
     */
    bool onEvent(const event::Event& event) override;
    
    /**
     * @brief Renders the ComboBox using the provided renderer.
     * @param renderer The renderer to draw with.
     */
    void render(Renderer& renderer) override;

private:
    void updateHeaderText();
    void positionDropdown();
    void handleHeaderClick();
    void handleListSelection(size_t index);
    bool isPointOutside(const Point<int32_t>& point) const;
};

} // namespace frqs::widget