
/**
 * @file button.cpp
 * @brief Implements the Button widget functionality.
 *
 * This file contains the implementation of the Button class, including event
 * handling for different states (normal, hover, pressed) and rendering logic.
 */

#include "widget/button.hpp"

namespace frqs::widget {

/**
 * @brief Private implementation details for the Button widget.
 * @internal
 */

struct Button::Impl {
    Point<int32_t> lastMousePos; ///< Stores the last known mouse position.
};

// ============================================================================
// BUTTON IMPLEMENTATION
// ============================================================================

/**
 * @brief Constructs a Button with the given text.
 */

Button::Button(const std::wstring& text)
    : Widget()
    , pImpl_(std::make_unique<Impl>())
    , text_(text)
{
    font_.size = 14.0f;
    font_.family = L"Segoe UI";
    font_.bold = false;
    setBackgroundColor(normalColor_);
}

/**
 * @brief Destroys the Button object.
 */

Button::~Button() = default;

/**
 * @brief Sets the button's text and invalidates the widget to trigger a repaint.
 */

void Button::setText(std::wstring_view text) {
    if (text_ == text) return;
    text_ = text;
    invalidate();
}

/**
 * @brief Sets the button's border properties and invalidates it for repaint.
 */

void Button::setBorder(const Color& color, float width) noexcept {
    borderColor_ = color;
    borderWidth_ = width;
    invalidate();
}

/**
 * @brief Updates the button's state and background color, then invalidates it.
 */

void Button::setState(State state) noexcept {
    if (state_ == state) return;
    state_ = state;
    setBackgroundColor(getCurrentColor());
    invalidate();
}

/**
 * @brief Sets the button's enabled state.
 */

void Button::setEnabled(bool enabled) noexcept {
    setState(enabled ? State::Normal : State::Disabled);
}

/**
 * @brief Retrieves the background color based on the current button state.
 * @internal
 */

Color Button::getCurrentColor() const noexcept {
    switch (state_) {
        case State::Normal:   return normalColor_;
        case State::Hovered:  return hoverColor_;
        case State::Pressed:  return pressedColor_;
        case State::Disabled: return disabledColor_;
        default:              return normalColor_;
    }
}

/**
 * @brief Checks if a point is within the button's rectangular bounds.
 * @internal
 */

bool Button::isPointInside(const Point<int32_t>& point) const noexcept {
    auto rect = getRect();
    return point.x >= rect.x && point.x < static_cast<int32_t>(rect.getRight()) &&
           point.y >= rect.y && point.y < static_cast<int32_t>(rect.getBottom());
}

/**
 * @brief Handles mouse events to update the button's state and trigger clicks.
 */

bool Button::onEvent(const event::Event& event) {
    if (!isEnabled()) return false;
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
                        // Button was clicked!
                        if (onClick_) {
                            onClick_();
                        }
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

/**
 * @brief Renders the button on the screen.
 */

void Button::render(Renderer& renderer) {
    if (!isVisible()) return;
    auto rect = getRect();
    auto bgColor = getCurrentColor();

    // Try to use extended renderer for rounded corners
    if (auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer)) {
        // Draw rounded rectangle button
        extRenderer->fillRoundedRect(rect, borderRadius_, borderRadius_, bgColor);
        
		// Draw border if present
        if (borderWidth_ > 0.0f && borderColor_.a > 0) {
            extRenderer->drawRoundedRect(
				rect, borderRadius_, 
				borderRadius_, borderColor_, 
				borderWidth_
			);
        }

        // Draw text
        if (!text_.empty()) {
            extRenderer->drawTextEx(
                text_, 
                rect, 
                textColor_,
                font_,
                render::TextAlign::Center,
                render::VerticalAlign::Middle
            );
        }
    } else {
        // Fallback to basic rendering
        renderer.fillRect(rect, bgColor);
        if (borderWidth_ > 0.0f && borderColor_.a > 0) {
            renderer.drawRect(rect, borderColor_, borderWidth_);
        }
        if (!text_.empty()) {
            renderer.drawText(text_, rect, textColor_);
        }
    }
}

} // namespace frqs::widget
