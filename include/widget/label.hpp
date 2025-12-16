#pragma once

#include "iwidget.hpp"
#include "render/renderer.hpp"

namespace frqs::widget {

// ============================================================================
// LABEL WIDGET (Read-only text display)
// ============================================================================

class Label : public Widget {
public:
    enum class Alignment : uint8_t {
        Left,
        Center,
        Right
    };

    enum class VerticalAlignment : uint8_t {
        Top,
        Middle,
        Bottom
    };

private:
    std::wstring text_;
    Color textColor_ = colors::Black;
    render::FontStyle font_;
    Alignment hAlign_ = Alignment::Left;
    VerticalAlignment vAlign_ = VerticalAlignment::Middle;
    uint32_t padding_ = 5;
    bool wordWrap_ = false;

public:
    explicit Label(const std::wstring& text = L"");
    ~Label() override = default;

    // Text content
    void setText(const std::wstring& text);
    const std::wstring& getText() const noexcept { return text_; }

    // Styling
    void setTextColor(const Color& color) noexcept;
    Color getTextColor() const noexcept { return textColor_; }

    void setFont(const render::FontStyle& font) noexcept { font_ = font; }
    const render::FontStyle& getFont() const noexcept { return font_; }

    void setFontSize(float size) noexcept { font_.size = size; }
    void setFontBold(bool bold) noexcept { font_.bold = bold; }
    void setFontItalic(bool italic) noexcept { font_.italic = italic; }

    // Alignment
    void setAlignment(Alignment align) noexcept { hAlign_ = align; }
    void setVerticalAlignment(VerticalAlignment align) noexcept { vAlign_ = align; }
    
    Alignment getAlignment() const noexcept { return hAlign_; }
    VerticalAlignment getVerticalAlignment() const noexcept { return vAlign_; }

    // Padding
    void setPadding(uint32_t padding) noexcept { padding_ = padding; }
    uint32_t getPadding() const noexcept { return padding_; }

    // Word wrap
    void setWordWrap(bool enable) noexcept { wordWrap_ = enable; }
    bool isWordWrapEnabled() const noexcept { return wordWrap_; }

    // Rendering
    void render(Renderer& renderer) override;

private:
    render::TextAlign toRenderAlign() const noexcept;
    render::VerticalAlign toRenderVerticalAlign() const noexcept;
};

} // namespace frqs::widget