// ============================================================================
// DEBUG CONFIGURATION
// 0: Off, 1: Click Only, 2: +Hover Enter, 3: +Hover Leave, 4: +Raw Details
// ============================================================================
#define FRQS_DEBUG_LEVEL 3

#include "widget/button.hpp"

// Setup Logging Macro
#if defined(FRQS_DEBUG_LEVEL) && FRQS_DEBUG_LEVEL > 0
    #include <print>
    #include <string>
    // Helper untuk konversi wstring ke string (biar println ga ngamuk)
    inline std::string to_string(const std::wstring& ws) {
        return std::string(ws.begin(), ws.end());
    }
    #define DBG_LOG(lvl, ...) \
        if constexpr (FRQS_DEBUG_LEVEL >= lvl) std::println(__VA_ARGS__)
#else
    #define DBG_LOG(lvl, ...) ((void)0)
#endif

namespace frqs::widget {

// ============================================================================
// BUTTON PIMPL
// ============================================================================

struct Button::Impl {
    Point<int32_t> lastMousePos;
};

// ============================================================================
// BUTTON IMPLEMENTATION
// ============================================================================

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

Button::~Button() = default;

void Button::setText(std::wstring_view text) {
    if (text_ == text) return;
    text_ = text;
    invalidate();
}

void Button::setBorder(const Color& color, float width) noexcept {
    borderColor_ = color;
    borderWidth_ = width;
    invalidate();
}

void Button::setState(State state) noexcept {
    if (state_ == state) return;
    state_ = state;
    setBackgroundColor(getCurrentColor());
    invalidate();
}

void Button::setEnabled(bool enabled) noexcept {
    setState(enabled ? State::Normal : State::Disabled);
}

Color Button::getCurrentColor() const noexcept {
    switch (state_) {
        case State::Normal:   return normalColor_;
        case State::Hovered:  return hoverColor_;
        case State::Pressed:  return pressedColor_;
        case State::Disabled: return disabledColor_;
        default:              return normalColor_;
    }
}

bool Button::isPointInside(const Point<int32_t>& point) const noexcept {
    auto rect = getRect();
    return point.x >= rect.x && point.x < static_cast<int32_t>(rect.getRight()) &&
           point.y >= rect.y && point.y < static_cast<int32_t>(rect.getBottom());
}

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
                            // LEVEL 1: Click Event (Paling penting)
                            #if defined(FRQS_DEBUG_LEVEL) && FRQS_DEBUG_LEVEL >= 1
                            DBG_LOG(1, "[Button] '{}' CLICKED!", to_string(text_));
                            #endif
                            
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
            // LEVEL 2: Enter Hover (Diagnosa Ghost Hover)
            #if defined(FRQS_DEBUG_LEVEL) && FRQS_DEBUG_LEVEL >= 2
            DBG_LOG(2, "[Button] '{}' ENTER Hover. Mouse: ({}, {})", 
                to_string(text_), mouseMove->position.x, mouseMove->position.y);
            #endif
            
            setState(State::Hovered);
            return true;
        } else if (!inside && state_ == State::Hovered) {
            // LEVEL 3: Leave Hover (Diagnosa salah koordinat)
            #if defined(FRQS_DEBUG_LEVEL) && FRQS_DEBUG_LEVEL >= 3
            DBG_LOG(3, "[Button] '{}' LEAVE Hover. Mouse: ({}, {})", 
                to_string(text_), mouseMove->position.x, mouseMove->position.y);
            #endif
            
            setState(State::Normal);
            return true;
        }
    }

    return Widget::onEvent(event);
}

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
            extRenderer->drawRoundedRect(rect, borderRadius_, borderRadius_, 
                                        borderColor_, borderWidth_);
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