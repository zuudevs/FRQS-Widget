#include "widget/label.hpp"

namespace frqs::widget {

// ============================================================================
// LABEL IMPLEMENTATION
// ============================================================================

Label::Label(const std::wstring& text)
    : Widget()
    , text_(text)
{
    font_.size = 14.0f;
    font_.family = L"Segoe UI";
    setBackgroundColor(colors::Transparent);
}

void Label::setText(const std::wstring& text) {
    if (text_ == text) return;
    text_ = text;
    invalidate();
}

void Label::setTextColor(const Color& color) noexcept {
    if (textColor_ == color) return;
    textColor_ = color;
    invalidate();
}

render::TextAlign Label::toRenderAlign() const noexcept {
    switch (hAlign_) {
        case Alignment::Left:   return render::TextAlign::Left;
        case Alignment::Center: return render::TextAlign::Center;
        case Alignment::Right:  return render::TextAlign::Right;
        default:                return render::TextAlign::Left;
    }
}

render::VerticalAlign Label::toRenderVerticalAlign() const noexcept {
    switch (vAlign_) {
        case VerticalAlignment::Top:    return render::VerticalAlign::Top;
        case VerticalAlignment::Middle: return render::VerticalAlign::Middle;
        case VerticalAlignment::Bottom: return render::VerticalAlign::Bottom;
        default:                        return render::VerticalAlign::Top;
    }
}

void Label::render(Renderer& renderer) {
    if (!isVisible()) return;

    // Render background (if not transparent)
    Widget::render(renderer);
    
    if (text_.empty()) return;

    auto rect = getRect();
    
    // Apply padding
    if (padding_ > 0) {
        rect = Rect<int32_t, uint32_t>(
            rect.x + static_cast<int32_t>(padding_),
            rect.y + static_cast<int32_t>(padding_),
            rect.w > padding_ * 2 ? rect.w - padding_ * 2 : 0,
            rect.h > padding_ * 2 ? rect.h - padding_ * 2 : 0
        );
    }
    
    if (rect.w == 0 || rect.h == 0) return;

    // Try to use extended renderer for better text rendering
    if (auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer)) {
        extRenderer->drawTextEx(
            text_, 
            rect, 
            textColor_,
            font_,
            toRenderAlign(),
            toRenderVerticalAlign()
        );
    } else {
        // Fallback to basic text rendering
        renderer.drawText(text_, rect, textColor_);
    }
}

} // namespace frqs::widget