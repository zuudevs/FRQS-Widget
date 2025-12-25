/**
 * @file combobox.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Implementation of the ComboBox widget.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "widget/combobox.hpp"
#include "widget/label.hpp"

namespace frqs::widget {

/**
 * @brief Private implementation details for the ComboBox widget.
 */
struct ComboBox::Impl {
    /**
     * @brief Holds the data adapter that provides items to the dropdown list.
     */
    std::shared_ptr<IListAdapter> adapter;
};

// ============================================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================================

ComboBox::ComboBox()
    : Widget()
    , pImpl_(std::make_unique<Impl>())
{
    setBackgroundColor(colors::Transparent);

    // Create the header button that displays the current selection and toggles the dropdown.
    headerButton_ = std::make_shared<Button>(L"Select...");
    headerButton_->setNormalColor(headerColor_);
    headerButton_->setHoverColor(headerHoverColor_);
    headerButton_->setBorder(dropdownBorderColor_, borderWidth_);
    headerButton_->setOnClick([this]() {
        handleHeaderClick();
    });
    addChild(headerButton_);

    // Create the dropdown list view. It is initially hidden and becomes visible
    // when the ComboBox is opened.
    dropdownList_ = std::make_shared<ListView>();
    dropdownList_->setVisible(false);
    dropdownList_->setBackgroundColor(dropdownBgColor_);
    dropdownList_->setItemHeight(itemHeight_);
    dropdownList_->setOnSelectionChanged([this](size_t index) {
        handleListSelection(index);
    });
    
    // The dropdown is added as a direct child. Its position will be managed
    // to appear as an overlay, drawn on top of sibling widgets.
    addChild(dropdownList_);
}

ComboBox::~ComboBox() = default;

// ========================================================================
// ADAPTER MANAGEMENT
// ========================================================================

void ComboBox::setAdapter(std::shared_ptr<IListAdapter> adapter) {
    pImpl_->adapter = adapter;
    
    if (dropdownList_) {
        dropdownList_->setAdapter(adapter);
    }
    
    // Reset selection and update the header text to reflect the new adapter.
    selectedIndex_ = size_t(-1);
    updateHeaderText();
}

std::shared_ptr<IListAdapter> ComboBox::getAdapter() const {
    return pImpl_->adapter;
}

void ComboBox::notifyDataChanged() {
    if (dropdownList_) {
        dropdownList_->notifyDataChanged();
    }
}

// ========================================================================
// SELECTION
// ========================================================================

void ComboBox::setSelectedIndex(size_t index) {
    // Validate the index against the adapter's item count.
    if (!pImpl_->adapter || index >= pImpl_->adapter->getCount()) {
        selectedIndex_ = size_t(-1);
        updateHeaderText();
        return;
    }
    
    if (selectedIndex_ == index) return;
    
    selectedIndex_ = index;
    
    if (dropdownList_) {
        dropdownList_->setSelectedIndex(index);
    }
    
    updateHeaderText();
    
    // Notify external listeners about the selection change.
    if (onSelectionChanged_) {
        onSelectionChanged_(index);
    }
}

// ========================================================================
// DROPDOWN CONTROL
// ========================================================================

void ComboBox::openDropdown() {
    if (isOpen_ || !pImpl_->adapter || pImpl_->adapter->getCount() == 0) {
        return;
    }
    
    isOpen_ = true;
    positionDropdown();
    dropdownList_->setVisible(true);
    invalidate();
}

void ComboBox::closeDropdown() {
    if (!isOpen_) return;
    
    isOpen_ = false;
    dropdownList_->setVisible(false);
    invalidate();
}

void ComboBox::toggleDropdown() {
    if (isOpen_) {
        closeDropdown();
    } else {
        openDropdown();
    }
}

// ========================================================================
// CONFIGURATION
// ========================================================================

void ComboBox::setItemHeight(uint32_t height) noexcept {
    itemHeight_ = height;
    if (dropdownList_) {
        dropdownList_->setItemHeight(height);
    }
}

void ComboBox::setDropdownBackgroundColor(const Color& color) noexcept {
    dropdownBgColor_ = color;
    if (dropdownList_) {
        dropdownList_->setBackgroundColor(color);
    }
}

// ========================================================================
// WIDGET OVERRIDES
// ========================================================================

void ComboBox::setRect(const Rect<int32_t, uint32_t>& rect) {
    Widget::setRect(rect);
    
    // Ensure the header button always fills the ComboBox's main rectangle.
    if (headerButton_) {
        headerButton_->setRect(rect);
    }
    
    // If the dropdown is open, its position must be recalculated relative to the new rect.
    if (isOpen_) {
        positionDropdown();
    }
}

bool ComboBox::onEvent(const event::Event& event) {
    // If the dropdown is open, we need to check for clicks outside of its bounds
    // to implement the auto-close behavior.
    if (isOpen_) {
        if (auto* btnEvt = std::get_if<event::MouseButtonEvent>(&event)) {
            if (btnEvt->action == event::MouseButtonEvent::Action::Press) {
                if (isPointOutside(btnEvt->position)) {
                    closeDropdown();
                    // Consume the event so other widgets don't react to this click.
                    return true;
                }
            }
        }
    }
    
    // Let the base Widget class propagate the event to children (header and list).
    // This allows the ListView to handle scrolling and selection.
    return Widget::onEvent(event);
}

void ComboBox::render(Renderer& renderer) {
    if (!isVisible()) return;
    
    // The header button is the primary visible component when the ComboBox is closed.
    if (headerButton_) {
        headerButton_->render(renderer);
    }
    
    // The dropdown list is rendered only when open. It is rendered after the
    // header button, so it appears on top of it and its siblings.
    if (isOpen_ && dropdownList_) {
        // Save the renderer's state if possible, to isolate drawing the dropdown.
        auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer);
        if (extRenderer) {
            extRenderer->save();
        }
        
        auto dropRect = dropdownList_->getRect();
        
        // Draw a border around the dropdown list.
        renderer.drawRect(dropRect, dropdownBorderColor_, borderWidth_);
        
        // Render the list view itself (items, scrollbar, etc.).
        dropdownList_->render(renderer);
        
        // Restore the renderer's state.
        if (extRenderer) {
            extRenderer->restore();
        }
    }
}

// ========================================================================
// INTERNAL HELPERS
// ========================================================================

/**
 * @brief Updates the text of the header button to reflect the current selection.
 * 
 * It creates a temporary view from the adapter to extract the display text for the
 * selected item. If the view is not a recognizable type (like a Label), it
 * falls back to displaying the item's index.
 */
void ComboBox::updateHeaderText() {
    if (!headerButton_) return;
    
    // If no item is selected or the adapter is missing, show a default prompt.
    if (!pImpl_->adapter || selectedIndex_ == size_t(-1) || 
        selectedIndex_ >= pImpl_->adapter->getCount()) {
        headerButton_->setText(L"Select...");
        return;
    }
    
    // Create a temporary widget view to get its textual representation.
    auto tempView = pImpl_->adapter->createView(selectedIndex_);
    pImpl_->adapter->updateView(selectedIndex_, tempView.get());
    
    // Attempt to cast the view to a Label to get its text.
    if (auto* label = dynamic_cast<Label*>(tempView.get())) {
        headerButton_->setText(label->getText());
    } else {
        // Fallback if the view isn't a simple label.
        headerButton_->setText(std::format(L"Item {}", selectedIndex_));
    }
}

/**
 * @brief Calculates and sets the position and size of the dropdown list.
 * 
 * The dropdown is positioned directly below the header button. Its height is
 * calculated based on the number of items and the configured item height, but
 * it is capped by `dropdownMaxHeight_`.
 */
void ComboBox::positionDropdown() {
    if (!dropdownList_ || !pImpl_->adapter) return;
    
    auto headerRect = getRect();
    
    size_t itemCount = pImpl_->adapter->getCount();
    uint32_t contentHeight = static_cast<uint32_t>(itemCount * itemHeight_);
    uint32_t dropdownHeight = std::min(contentHeight, dropdownMaxHeight_);
    
    // Position dropdown to align with the header's left edge and appear directly below it.
    int32_t dropX = headerRect.x;
    int32_t dropY = headerRect.y + static_cast<int32_t>(headerRect.h);
    uint32_t dropW = headerRect.w;
    
    dropdownList_->setRect(Rect<int32_t, uint32_t>(
        dropX, dropY, dropW, dropdownHeight
    ));
}

/**
 * @brief Handles a click on the header button, toggling the dropdown's visibility.
 */
void ComboBox::handleHeaderClick() {
    toggleDropdown();
}

/**
 * @brief Handles a selection change from the dropdown ListView.
 * @param index The index of the item that was selected in the list.
 */
void ComboBox::handleListSelection(size_t index) {
    setSelectedIndex(index);
    closeDropdown();
}

/**
 * @brief Checks if a point is outside the combined area of the header and the dropdown list.
 * @param point The point to check, typically the cursor position on a mouse click.
 * @return True if the point is outside both the header and the (if open) dropdown.
 */
bool ComboBox::isPointOutside(const Point<int32_t>& point) const {
    // Check if the point is inside the main header button.
    if (getRect().contain(point)) {
        return false;
    }
    
    // If the dropdown is open, also check if the point is inside its bounds.
    if (isOpen_ && dropdownList_) {
        if (dropdownList_->getRect().contain(point)) {
            return false;
        }
    }
    
    // If the point is not in the header or the dropdown, it's outside.
    return true;
}

} // namespace frqs::widget