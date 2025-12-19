// include/widget/checkbox.hpp
#pragma once

#include "iwidget.hpp"
#include <functional>
#include "render/renderer.hpp"

namespace frqs::widget {

// ============================================================================
// CHECKBOX WIDGET (Boolean toggle with label)
// ============================================================================

class CheckBox : public Widget {
public:
    using ChangedCallback = std::function<void(bool)>;

    enum class State : uint8_t {
        Normal,
        Hovered,
        Pressed,
        Disabled
    };

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;

    // State
    bool checked_ = false;
    State state_ = State::Normal;
    bool enabled_ = true;
    
    // Text
    std::wstring text_;
    Color textColor_ = colors::Black;
    render::FontStyle font_;
    
    // Visual properties
    float boxSize_ = 18.0f;
    float spacing_ = 8.0f;
    float borderRadius_ = 3.0f;
    
    // Colors
    Color boxColor_ = colors::White;
    Color boxBorderColor_ = Color(189, 195, 199);
    Color checkedColor_ = Color(52, 152, 219);
    Color hoverBorderColor_ = Color(52, 152, 219);
    Color disabledColor_ = Color(189, 195, 199);
    
    // Callback
    ChangedCallback onChanged_;

public:
    explicit CheckBox(const std::wstring& text = L"");
    ~CheckBox() override;

    // State
    void setChecked(bool checked);
    bool isChecked() const noexcept { return checked_; }
    void toggle() { setChecked(!checked_); }

    void setEnabled(bool enabled) noexcept;
    bool isEnabled() const noexcept { return enabled_; }

    // Text
    void setText(const std::wstring& text);
    const std::wstring& getText() const noexcept { return text_; }

    void setTextColor(const Color& color) noexcept { textColor_ = color; }
    Color getTextColor() const noexcept { return textColor_; }

    void setFont(const render::FontStyle& font) noexcept { font_ = font; }
    void setFontSize(float size) noexcept { font_.size = size; }

    // Visual properties
    void setBoxSize(float size) noexcept { boxSize_ = size; }
    void setSpacing(float spacing) noexcept { spacing_ = spacing; }
    void setBorderRadius(float radius) noexcept { borderRadius_ = radius; }

    // Colors
    void setBoxColor(const Color& color) noexcept { boxColor_ = color; }
    void setCheckedColor(const Color& color) noexcept { checkedColor_ = color; }
    void setHoverBorderColor(const Color& color) noexcept { hoverBorderColor_ = color; }

    // Callback
    void setOnChanged(ChangedCallback callback) { onChanged_ = std::move(callback); }

    // Event handling
    bool onEvent(const event::Event& event) override;

    // Rendering
    void render(Renderer& renderer) override;

private:
    void setState(State state) noexcept;
    Color getCurrentBorderColor() const noexcept;
    bool isPointInside(const Point<int32_t>& point) const noexcept;
    void notifyChanged();
};

} // namespace frqs::widget