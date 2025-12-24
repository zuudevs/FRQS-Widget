#pragma once

#include "iwidget.hpp"
#include "render/renderer.hpp"
#include <functional>

namespace frqs::widget {

// ============================================================================
// BUTTON WIDGET (Clickable button with states)
// ============================================================================

class Button : public Widget {
public:
    using ClickCallback = std::function<void()>;

    enum class State : uint8_t {
        Normal,
        Hovered,
        Pressed,
        Disabled
    };

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;

    // Visual state
    State state_ = State::Normal;
    
    // Text
    std::wstring text_;
    Color textColor_ = colors::White;
    render::FontStyle font_;
    
    // Colors per state
    Color normalColor_ = Color(52, 152, 219);     // Blue
    Color hoverColor_ = Color(41, 128, 185);      // Darker blue
    Color pressedColor_ = Color(21, 101, 192);    // Even darker
    Color disabledColor_ = Color(149, 165, 166);  // Gray
    
    // Border
    float borderRadius_ = 4.0f;
    Color borderColor_ = colors::Transparent;
    float borderWidth_ = 0.0f;
    
    // Callback
    ClickCallback onClick_;

public:
    explicit Button(const std::wstring& text = L"Button");
    ~Button() override;

    // Text
    void setText(std::wstring_view text);
    const std::wstring& getText() const noexcept { return text_; }

    void setTextColor(const Color& color) noexcept { textColor_ = color; }
    Color getTextColor() const noexcept { return textColor_; }

    void setFont(const render::FontStyle& font) noexcept { font_ = font; }
    void setFontSize(float size) noexcept { font_.size = size; }

    // State colors
    void setNormalColor(const Color& color) noexcept { normalColor_ = color; }
    void setHoverColor(const Color& color) noexcept { hoverColor_ = color; }
    void setPressedColor(const Color& color) noexcept { pressedColor_ = color; }
    void setDisabledColor(const Color& color) noexcept { disabledColor_ = color; }

    // Border
    void setBorderRadius(float radius) noexcept { borderRadius_ = radius; }
    void setBorder(const Color& color, float width) noexcept;

    // State
    void setState(State state) noexcept;
    State getState() const noexcept { return state_; }
    
    void setEnabled(bool enabled) noexcept;
    bool isEnabled() const noexcept { return state_ != State::Disabled; }

    // Callback
    void setOnClick(ClickCallback callback) { onClick_ = std::move(callback); }

    // Event handling
    bool onEvent(const event::Event& event) override;

    // Rendering
    void render(Renderer& renderer) override;

private:
    Color getCurrentColor() const noexcept;
    bool isPointInside(const Point<int32_t>& point) const noexcept;
};

} // namespace frqs::widget