/**
 * @file label.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines the Label widget for displaying read-only text.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "iwidget.hpp"
#include "render/renderer.hpp"

namespace frqs::widget {

/**
 * @brief A widget for displaying read-only text.
 * @details The Label widget provides a simple way to display text with various
 * styling options, including alignment, color, font, and word wrapping.
 */
class Label : public Widget {
public:
    /**
     * @brief Specifies the horizontal alignment of the text within the label's bounds.
     */
    enum class Alignment : uint8_t {
        Left,   ///< Align text to the left.
        Center, ///< Align text to the center.
        Right   ///< Align text to the right.
    };

    /**
     * @brief Specifies the vertical alignment of the text within the label's bounds.
     */
    enum class VerticalAlignment : uint8_t {
        Top,    ///< Align text to the top.
        Middle, ///< Align text to the middle.
        Bottom  ///< Align text to the bottom.
    };

private:
    std::wstring text_; ///< The text content of the label.
    Color textColor_ = colors::Black; ///< The color of the text.
    render::FontStyle font_; ///< The font style used for rendering the text.
    Alignment hAlign_ = Alignment::Left; ///< Horizontal text alignment.
    VerticalAlignment vAlign_ = VerticalAlignment::Middle; ///< Vertical text alignment.
    uint32_t padding_ = 5; ///< Padding around the text within the widget bounds.
    bool wordWrap_ = false; ///< Flag to enable or disable word wrapping.

public:
    /**
     * @brief Constructs a new Label object.
     * @param text The initial text to display.
     */
    explicit Label(const std::wstring& text = L"");
    
    /**
     * @brief Default destructor.
     */
    ~Label() override = default;

    /**
     * @brief Sets the text content of the label.
     * @param text The new text to display.
     */
    void setText(std::wstring_view text);

    /**
     * @brief Gets the current text content of the label.
     * @return A constant reference to the label's text.
     */
    const std::wstring& getText() const noexcept { return text_; }

    /**
     * @brief Sets the color of the text.
     * @param color The new text color.
     */
    void setTextColor(const Color& color) noexcept;

    /**
     * @brief Gets the current text color.
     * @return The current color of the text.
     */
    Color getTextColor() const noexcept { return textColor_; }

    /**
     * @brief Sets the font style for the text.
     * @param font The new font style.
     */
    void setFont(const render::FontStyle& font) noexcept { font_ = font; }

    /**
     * @brief Gets the current font style.
     * @return A constant reference to the current font style.
     */
    const render::FontStyle& getFont() const noexcept { return font_; }

    /**
     * @brief Sets the font size.
     * @param size The new font size.
     */
    void setFontSize(float size) noexcept { font_.size = size; }

    /**
     * @brief Sets the font weight to bold or normal.
     * @param bold `true` for bold, `false` for normal.
     */
    void setFontBold(bool bold) noexcept { font_.bold = bold; }

    /**
     * @brief Sets the font style to italic or normal.
     * @param italic `true` for italic, `false` for normal.
     */
    void setFontItalic(bool italic) noexcept { font_.italic = italic; }

    /**
     * @brief Sets the horizontal alignment of the text.
     * @param align The new horizontal alignment.
     */
    void setAlignment(Alignment align) noexcept { hAlign_ = align; }

    /**
     * @brief Sets the vertical alignment of the text.
     * @param align The new vertical alignment.
     */
    void setVerticalAlignment(VerticalAlignment align) noexcept { vAlign_ = align; }
    
    /**
     * @brief Gets the current horizontal text alignment.
     * @return The current horizontal alignment.
     */
    Alignment getAlignment() const noexcept { return hAlign_; }

    /**
     * @brief Gets the current vertical text alignment.
     * @return The current vertical alignment.
     */
    VerticalAlignment getVerticalAlignment() const noexcept { return vAlign_; }

    /**
     * @brief Sets the padding around the text.
     * @param padding The padding value in pixels.
     */
    void setPadding(uint32_t padding) noexcept { padding_ = padding; }

    /**
     * @brief Gets the current padding value.
     * @return The padding in pixels.
     */
    uint32_t getPadding() const noexcept { return padding_; }

    /**
     * @brief Enables or disables word wrapping.
     * @param enable `true` to enable word wrapping, `false` to disable it.
     */
    void setWordWrap(bool enable) noexcept { wordWrap_ = enable; }

    /**
     * @brief Checks if word wrapping is enabled.
     * @return `true` if word wrapping is enabled, `false` otherwise.
     */
    bool isWordWrapEnabled() const noexcept { return wordWrap_; }

    /**
     * @brief Renders the label widget.
     * @details This method is called by the rendering system to draw the widget.
     * @param renderer The renderer to use for drawing.
     */
    void render(Renderer& renderer) override;

private:
    /**
     * @brief Converts the internal horizontal alignment enum to the renderer's equivalent.
     * @return The renderer-specific text alignment value.
     */
    render::TextAlign toRenderAlign() const noexcept;

    /**
     * @brief Converts the internal vertical alignment enum to the renderer's equivalent.
     * @return The renderer-specific vertical alignment value.
     */
    render::VerticalAlign toRenderVerticalAlign() const noexcept;
};

} // namespace frqs::widget