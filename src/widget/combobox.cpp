// src/widget/combobox.cpp
#include "widget/combobox.hpp"
#include "widget/label.hpp"

namespace frqs::widget {

// ============================================================================
// PIMPL
// ============================================================================

struct ComboBox::Impl {
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

    // Create header button
    headerButton_ = std::make_shared<Button>(L"Select...");
    headerButton_->setNormalColor(headerColor_);
    headerButton_->setHoverColor(headerHoverColor_);
    headerButton_->setBorder(dropdownBorderColor_, borderWidth_);
    headerButton_->setOnClick([this]() {
        handleHeaderClick();
    });
    
    addChild(headerButton_);

    // Create dropdown list (initially hidden)
    dropdownList_ = std::make_shared<ListView>();
    dropdownList_->setVisible(false);
    dropdownList_->setBackgroundColor(dropdownBgColor_);
    dropdownList_->setItemHeight(itemHeight_);
    dropdownList_->setOnSelectionChanged([this](size_t index) {
        handleListSelection(index);
    });
    
    // Note: We add the dropdown to the widget tree, but it will be
    // positioned to appear as an "overlay". In a full overlay system,
    // this would be added to a Window-level overlay list instead.
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
    
    // Reset selection
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
    
    // Position header button to fill ComboBox rect
    if (headerButton_) {
        headerButton_->setRect(rect);
    }
    
    // Reposition dropdown if open
    if (isOpen_) {
        positionDropdown();
    }
}

bool ComboBox::onEvent(const event::Event& event) {
    // If dropdown is open, handle clicks outside to close
    if (isOpen_) {
        if (auto* btnEvt = std::get_if<event::MouseButtonEvent>(&event)) {
            if (btnEvt->action == event::MouseButtonEvent::Action::Press) {
                if (isPointOutside(btnEvt->position)) {
                    closeDropdown();
                    return true;
                }
            }
        }
    }
    
    // Forward to children (header button, dropdown list)
    return Widget::onEvent(event);
}

void ComboBox::render(Renderer& renderer) {
    if (!isVisible()) return;
    
    // Render header button
    if (headerButton_) {
        headerButton_->render(renderer);
    }
    
    // Render dropdown (if open)
    // Note: In a full overlay system, this would be rendered by Window
    // at the end of the frame (top-most Z-order)
    if (isOpen_ && dropdownList_) {
        // Save render state
        auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer);
        if (extRenderer) {
            extRenderer->save();
        }
        
        // Render dropdown with border
        auto dropRect = dropdownList_->getRect();
        
        // Border
        renderer.drawRect(dropRect, dropdownBorderColor_, borderWidth_);
        
        // Content
        dropdownList_->render(renderer);
        
        // Restore render state
        if (extRenderer) {
            extRenderer->restore();
        }
    }
}

// ========================================================================
// INTERNAL
// ========================================================================

void ComboBox::updateHeaderText() {
    if (!headerButton_) return;
    
    if (!pImpl_->adapter || selectedIndex_ == size_t(-1) || 
        selectedIndex_ >= pImpl_->adapter->getCount()) {
        headerButton_->setText(L"Select...");
        return;
    }
    
    // Create a temporary view to get the display text
    auto tempView = pImpl_->adapter->createView(selectedIndex_);
    pImpl_->adapter->updateView(selectedIndex_, tempView.get());
    
    // Try to extract text from the view
    if (auto* label = dynamic_cast<Label*>(tempView.get())) {
        headerButton_->setText(label->getText());
    } else {
        // Fallback: show index
        headerButton_->setText(std::format(L"Item {}", selectedIndex_));
    }
}

void ComboBox::positionDropdown() {
    if (!dropdownList_ || !pImpl_->adapter) return;
    
    auto headerRect = getRect();
    
    // Calculate dropdown dimensions
    size_t itemCount = pImpl_->adapter->getCount();
    uint32_t contentHeight = static_cast<uint32_t>(itemCount * itemHeight_);
    uint32_t dropdownHeight = std::min(contentHeight, dropdownMaxHeight_);
    
    // Position dropdown below header
    int32_t dropX = headerRect.x;
    int32_t dropY = headerRect.y + static_cast<int32_t>(headerRect.h);
    uint32_t dropW = headerRect.w;
    
    dropdownList_->setRect(Rect<int32_t, uint32_t>(
        dropX, dropY, dropW, dropdownHeight
    ));
}

void ComboBox::handleHeaderClick() {
    toggleDropdown();
}

void ComboBox::handleListSelection(size_t index) {
    setSelectedIndex(index);
    closeDropdown();
}

bool ComboBox::isPointOutside(const Point<int32_t>& point) const {
    // Check if point is inside header
    auto headerRect = getRect();
    bool insideHeader = 
        point.x >= headerRect.x && 
        point.x < static_cast<int32_t>(headerRect.getRight()) &&
        point.y >= headerRect.y && 
        point.y < static_cast<int32_t>(headerRect.getBottom());
    
    if (insideHeader) return false;
    
    // Check if point is inside dropdown
    if (isOpen_ && dropdownList_) {
        auto dropRect = dropdownList_->getRect();
        bool insideDropdown = 
            point.x >= dropRect.x && 
            point.x < static_cast<int32_t>(dropRect.getRight()) &&
            point.y >= dropRect.y && 
            point.y < static_cast<int32_t>(dropRect.getBottom());
        
        if (insideDropdown) return false;
    }
    
    return true;
}

} // namespace frqs::widget