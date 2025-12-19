// src/widget/checkbox.cpp
#include "widget/checkbox.hpp"

namespace frqs::widget {

// ============================================================================
// CHECKBOX PIMPL
// ============================================================================

struct CheckBox::Impl {
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

void CheckBox::setState(State state) noexcept {
    if (state_ == state) return;
    state_ = state;
    invalidate();
}

Color CheckBox::getCurrentBorderColor() const noexcept {
    if (!enabled_) return disabledColor_;
    if (state_ == State::Hovered || state_ == State::Pressed) return hoverBorderColor_;
    if (checked_) return checkedColor_;
    return boxBorderColor_;
}

bool CheckBox::isPointInside(const Point<int32_t>& point) const noexcept {
    auto rect = getRect();
    return point.x >= rect.x && point.x < static_cast<int32_t>(rect.getRight()) &&
           point.y >= rect.y && point.y < static_cast<int32_t>(rect.getBottom());
}

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

    // Handle mouse button events
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
                        // Toggle checked state
                        toggle();
                        setState(State::Hovered);
                    } else {
                        setState(State::Normal);
                    }
                    return true;
                }
            }
        }
    }
    
    // Handle mouse move events
    if (auto* mouseMove = std::get_if<event::MouseMoveEvent>(&event)) {
        pImpl_->lastMousePos = mouseMove->position;
        
        bool inside = isPointInside(mouseMove->position);
        
        if (state_ == State::Pressed) {
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
    
    // Render background (if not transparent)
    Widget::render(renderer);
    
    // Calculate box position (left side)
    int32_t boxX = rect.x;
    int32_t boxY = rect.y + static_cast<int32_t>((rect.h - static_cast<uint32_t>(boxSize_)) / 2);
    
    Rect<int32_t, uint32_t> boxRect(
        boxX, boxY, 
        static_cast<uint32_t>(boxSize_), 
        static_cast<uint32_t>(boxSize_)
    );
    
    // Try to use extended renderer for rounded corners
    if (auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer)) {
        // Draw box background
        extRenderer->fillRoundedRect(boxRect, borderRadius_, borderRadius_, boxColor_);
        
        // Draw box border
        Color borderColor = getCurrentBorderColor();
        extRenderer->drawRoundedRect(boxRect, borderRadius_, borderRadius_, borderColor, 2.0f);
        
        // Draw checkmark if checked
        if (checked_) {
            // Draw checkmark using lines
            float centerX = boxX + boxSize_ / 2.0f;
            float centerY = boxY + boxSize_ / 2.0f;
            float size = boxSize_ * 0.6f;
            
            // Checkmark path: two lines forming a check
            Point<int32_t> p1(
                static_cast<int32_t>(centerX - size * 0.3f),
                static_cast<int32_t>(centerY)
            );
            Point<int32_t> p2(
                static_cast<int32_t>(centerX - size * 0.1f),
                static_cast<int32_t>(centerY + size * 0.3f)
            );
            Point<int32_t> p3(
                static_cast<int32_t>(centerX + size * 0.4f),
                static_cast<int32_t>(centerY - size * 0.4f)
            );
            
            extRenderer->drawLine(p1, p2, checkedColor_, 2.5f);
            extRenderer->drawLine(p2, p3, checkedColor_, 2.5f);
        }
        
        // Draw text label
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
        // Fallback to basic rendering
        renderer.fillRect(boxRect, boxColor_);
        renderer.drawRect(boxRect, getCurrentBorderColor(), 2.0f);
        
        if (checked_) {
            // Simple filled rect for checkmark (fallback)
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