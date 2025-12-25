/**
 * @file checkbox.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Implementation of the CheckBox widget.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "widget/checkbox.hpp"

namespace frqs::widget {

/**
 * @brief Private implementation details for the CheckBox widget.
 */
struct CheckBox::Impl {
    /**
     * @brief Stores the last known position of the mouse cursor.
     * Used to handle hover state changes correctly.
     */
    Point<int32_t> lastMousePos;
};

// ============================================================================
// CHECKBOX IMPLEMENTATION
// ============================================================================

CheckBox::CheckBox(const std::wstring& text)
    : Widget()
    , pImpl_(std::make_unique<Impl>())
    , text_(text)
{
    font_.size = 14.0f;
    font_.family = L"Segoe UI";
    setBackgroundColor(colors::Transparent);
}

CheckBox::~CheckBox() = default;

void CheckBox::setChecked(bool checked) {
    if (checked_ == checked) return;
    
    checked_ = checked;
    notifyChanged();
    invalidate();
}

void CheckBox::setEnabled(bool enabled) noexcept {
    if (enabled_ == enabled) return;
    
    enabled_ = enabled;
    setState(enabled ? State::Normal : State::Disabled);
}

void CheckBox::setText(const std::wstring& text) {
    if (text_ == text) return;
    text_ = text;
    invalidate();
}

/**
 * @brief Sets the internal state of the checkbox and requests a redraw.
 * @param state The new state to set.
 */
void CheckBox::setState(State state) noexcept {
    if (state_ == state) return;
    state_ = state;
    invalidate();
}

/**
 * @brief Determines the appropriate border color based on the current state.
 * @return The color to be used for the checkbox border.
 */
Color CheckBox::getCurrentBorderColor() const noexcept {
    if (!enabled_) return disabledColor_;
    if (state_ == State::Hovered || state_ == State::Pressed) return hoverBorderColor_;
    if (checked_) return checkedColor_;
    return boxBorderColor_;
}

/**
 * @brief Checks if a given point is within the widget's bounds.
 * @param point The point to check.
 * @return True if the point is inside the widget, false otherwise.
 */
bool CheckBox::isPointInside(const Point<int32_t>& point) const noexcept {
    auto rect = getRect();
    return point.x >= rect.x && point.x < static_cast<int32_t>(rect.getRight()) &&
           point.y >= rect.y && point.y < static_cast<int32_t>(rect.getBottom());
}

/**
 * @brief Invokes the onChanged_ callback if it is set.
 * 
 * Exceptions thrown by the callback are caught and ignored to prevent
 * the application from crashing.
 */
void CheckBox::notifyChanged() {
    if (onChanged_) {
        try {
            onChanged_(checked_);
        } catch (...) {
            // Swallow callback exceptions
        }
    }
}

bool CheckBox::onEvent(const event::Event& event) {
    if (!enabled_) return false;

    // Handle mouse button events for press, release, and toggle logic
    if (auto* mouseBtn = std::get_if<event::MouseButtonEvent>(&event)) {
        if (mouseBtn->button == event::MouseButtonEvent::Button::Left) {
            if (mouseBtn->action == event::MouseButtonEvent::Action::Press) {
                if (isPointInside(mouseBtn->position)) {
                    setState(State::Pressed);
                    return true;
                }
            } else if (mouseBtn->action == event::MouseButtonEvent::Action::Release) {
                if (state_ == State::Pressed) {
                    if (isPointInside(mouseBtn->position)) {
                        // Toggle the checked state on release inside the widget
                        toggle();
                        setState(State::Hovered);
                    } else {
                        // Mouse was released outside, revert to normal state
                        setState(State::Normal);
                    }
                    return true;
                }
            }
        }
    }
    
    // Handle mouse move events for hover effects
    if (auto* mouseMove = std::get_if<event::MouseMoveEvent>(&event)) {
        pImpl_->lastMousePos = mouseMove->position;
        
        bool inside = isPointInside(mouseMove->position);
        
        if (state_ == State::Pressed) {
            // If pressed, hover state doesn't change until release
            return true;
        } else if (inside && state_ == State::Normal) {
            setState(State::Hovered);
            return true;
        } else if (!inside && state_ == State::Hovered) {
            setState(State::Normal);
            return true;
        }
    }

    return Widget::onEvent(event);
}

void CheckBox::render(Renderer& renderer) {
    if (!isVisible()) return;

    auto rect = getRect();
    
    // Render the widget's background color (usually transparent for a checkbox)
    Widget::render(renderer);
    
    // Calculate the position and size of the check box square
    int32_t boxX = rect.x;
    int32_t boxY = rect.y + static_cast<int32_t>((rect.h - static_cast<uint32_t>(boxSize_)) / 2);
    
    Rect<int32_t, uint32_t> boxRect(
        boxX, boxY, 
        static_cast<uint32_t>(boxSize_), 
        static_cast<uint32_t>(boxSize_)
    );
    
    // Use the extended renderer for anti-aliased rounded corners if available
    if (auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer)) {
        // Draw the main box background
        extRenderer->fillRoundedRect(boxRect, borderRadius_, borderRadius_, boxColor_);
        
        // Draw the box border with a color determined by the current state
        Color borderColor = getCurrentBorderColor();
        extRenderer->drawRoundedRect(boxRect, borderRadius_, borderRadius_, borderColor, 2.0f);
        
        // Draw the checkmark symbol if the box is checked
        if (checked_) {
            // The checkmark is drawn as two connected lines
            float centerX = boxX + boxSize_ / 2.0f;
            float centerY = boxY + boxSize_ / 2.0f;
            float size = boxSize_ * 0.6f;
            
            Point<int32_t> p1(static_cast<int32_t>(centerX - size * 0.3f), static_cast<int32_t>(centerY));
            Point<int32_t> p2(static_cast<int32_t>(centerX - size * 0.1f), static_cast<int32_t>(centerY + size * 0.3f));
            Point<int32_t> p3(static_cast<int32_t>(centerX + size * 0.4f), static_cast<int32_t>(centerY - size * 0.4f));
            
            extRenderer->drawLine(p1, p2, checkedColor_, 2.5f);
            extRenderer->drawLine(p2, p3, checkedColor_, 2.5f);
        }
        
        // Draw the text label to the right of the check box
        if (!text_.empty()) {
            int32_t textX = boxX + static_cast<int32_t>(boxSize_ + spacing_);
            Rect<int32_t, uint32_t> textRect(
                textX,
                rect.y,
                rect.w > static_cast<uint32_t>(boxSize_ + spacing_) 
                    ? rect.w - static_cast<uint32_t>(boxSize_ + spacing_)
                    : 0,
                rect.h
            );
            
            // Use different colors for enabled/disabled states
            extRenderer->drawTextEx(
                text_,
                textRect,
                enabled_ ? textColor_ : disabledColor_,
                font_,
                render::TextAlign::Left,
                render::VerticalAlign::Middle
            );
        }
    } else {
        // Fallback to basic rendering if the extended renderer is not available
        renderer.fillRect(boxRect, boxColor_);
        renderer.drawRect(boxRect, getCurrentBorderColor(), 2.0f);
        
        if (checked_) {
            // Fallback checkmark is a simple filled rectangle
            Rect<int32_t, uint32_t> checkRect(
                boxX + static_cast<int32_t>(boxSize_ * 0.25f),
                boxY + static_cast<int32_t>(boxSize_ * 0.25f),
                static_cast<uint32_t>(boxSize_ * 0.5f),
                static_cast<uint32_t>(boxSize_ * 0.5f)
            );
            renderer.fillRect(checkRect, checkedColor_);
        }
        
        if (!text_.empty()) {
            int32_t textX = boxX + static_cast<int32_t>(boxSize_ + spacing_);
            Rect<int32_t, uint32_t> textRect(
                textX, rect.y,
                rect.w > static_cast<uint32_t>(boxSize_ + spacing_)
                    ? rect.w - static_cast<uint32_t>(boxSize_ + spacing_)
                    : 0,
                rect.h
            );
            renderer.drawText(text_, textRect, enabled_ ? textColor_ : disabledColor_);
        }
    }
}

} // namespace frqs::widget