/**
 * @file renderer.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines the extended renderer interface and related graphics utilities.
 * @version 0.1
 * @date 2025-12-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

// Note: The Renderer interface is already defined in widget/iwidget.hpp
// This header provides additional rendering utilities

#include "widget/iwidget.hpp"
#include <string>

namespace frqs::render {

// ============================================================================
// RENDERING UTILITIES
// ============================================================================

/**
 * @struct ColorF
 * @brief Represents a color with floating-point components in the range [0.0, 1.0].
 * 
 * This is often required by graphics APIs for color representation.
 */
struct ColorF {
    float r, g, b, a;

    /**
     * @brief Constructs a ColorF from a standard `widget::Color`.
     * @param color The `widget::Color` with 8-bit integer components.
     */
    constexpr ColorF(const widget::Color& color) noexcept
        : r(color.r / 255.0f)
        , g(color.g / 255.0f)
        , b(color.b / 255.0f)
        , a(color.a / 255.0f) {}

    /**
     * @brief Constructs a ColorF from individual float components.
     * @param r Red component [0.0, 1.0].
     * @param g Green component [0.0, 1.0].
     * @param b Blue component [0.0, 1.0].
     * @param a Alpha component [0.0, 1.0], defaults to 1.0 (opaque).
     */
    constexpr ColorF(float r, float g, float b, float a = 1.0f) noexcept
        : r(r), g(g), b(b), a(a) {}
};

// ============================================================================
// TEXT ALIGNMENT
// ============================================================================

/**
 * @enum TextAlign
 * @brief Specifies horizontal alignment for text.
 */
enum class TextAlign : uint8_t {
    Left,    ///< Align text to the left.
    Center,  ///< Center text horizontally.
    Right,   ///< Align text to the right.
    Justify  ///< Stretch lines so that each line has equal width (not always supported).
};

/**
 * @enum VerticalAlign
 * @brief Specifies vertical alignment for text.
 */
enum class VerticalAlign : uint8_t {
    Top,    ///< Align text to the top.
    Middle, ///< Center text vertically.
    Bottom  ///< Align text to the bottom.
};

// ============================================================================
// FONT STYLE
// ============================================================================

/**
 * @struct FontStyle
 * @brief Describes the properties of a font used for rendering text.
 */
struct FontStyle {
    std::wstring family = L"Segoe UI"; ///< Font family name.
    float size = 14.0f;               ///< Font size in device-independent pixels.
    bool bold = false;                ///< `true` if the font is bold.
    bool italic = false;              ///< `true` if the font is italic.
    bool underline = false;           ///< `true` if the text should be underlined.
    bool strikethrough = false;       ///< `true` if the text should have a strikethrough.
    
    /**
     * @brief Compares two FontStyle objects for equality.
     * @param other The other FontStyle to compare against.
     * @return `true` if all properties are identical, `false` otherwise.
     */
    bool operator==(const FontStyle& other) const noexcept {
        return family == other.family &&
               size == other.size &&
               bold == other.bold &&
               italic == other.italic &&
               underline == other.underline &&
               strikethrough == other.strikethrough;
    }
};

// ============================================================================
// BRUSH TYPES
// ============================================================================

/**
 * @enum BrushType
 * @brief Defines the type of brush used for filling shapes.
 */
enum class BrushType : uint8_t {
    Solid,          ///< A solid color brush.
    LinearGradient, ///< A linear gradient brush.
    RadialGradient, ///< A radial gradient brush.
    Bitmap          ///< A brush that tiles a bitmap image.
};

// ============================================================================
// EXTENDED RENDERER INTERFACE (Optional)
// ============================================================================

/**
 * @class IExtendedRenderer
 * @brief An optional, extended interface for renderers that support more advanced drawing operations.
 * 
 * This interface inherits from `widget::Renderer` and adds methods for drawing
 * complex shapes, handling advanced text layout, and more granular state management.
 */
class IExtendedRenderer : public widget::Renderer {
public:
    // Advanced drawing
    /**
     * @brief Draws a line between two points.
     * @param start The starting point.
     * @param end The ending point.
     * @param color The color of the line.
     * @param strokeWidth The thickness of the line.
     */
    virtual void drawLine(const widget::Point<int32_t>& start,
                         const widget::Point<int32_t>& end,
                         const widget::Color& color,
                         float strokeWidth = 1.0f) = 0;

    /**
     * @brief Draws the outline of an ellipse.
     * @param rect The bounding rectangle of the ellipse.
     * @param color The color of the outline.
     * @param strokeWidth The thickness of the outline.
     */
    virtual void drawEllipse(const widget::Rect<int32_t, uint32_t>& rect,
                            const widget::Color& color,
                            float strokeWidth = 1.0f) = 0;

    /**
     * @brief Fills the interior of an ellipse.
     * @param rect The bounding rectangle of the ellipse.
     * @param color The fill color.
     */
    virtual void fillEllipse(const widget::Rect<int32_t, uint32_t>& rect,
                            const widget::Color& color) = 0;

    /**
     * @brief Draws the outline of a rounded rectangle.
     * @param rect The bounding rectangle.
     * @param radiusX The horizontal radius of the corners.
     * @param radiusY The vertical radius of the corners.
     * @param color The color of the outline.
     * @param strokeWidth The thickness of the outline.
     */
    virtual void drawRoundedRect(const widget::Rect<int32_t, uint32_t>& rect,
                                float radiusX, float radiusY,
                                const widget::Color& color,
                                float strokeWidth = 1.0f) = 0;

    /**
     * @brief Fills the interior of a rounded rectangle.
     * @param rect The bounding rectangle.
     * @param radiusX The horizontal radius of the corners.
     * @param radiusY The vertical radius of the corners.
     * @param color The fill color.
     */
    virtual void fillRoundedRect(const widget::Rect<int32_t, uint32_t>& rect,
                                float radiusX, float radiusY,
                                const widget::Color& color) = 0;

    // Text with advanced options
    /**
     * @brief Draws text with advanced alignment and font styling.
     * @param text The text to draw.
     * @param rect The layout rectangle for the text.
     * @param color The color of the text.
     * @param font The font style to use.
     * @param halign Horizontal alignment.
     * @param valign Vertical alignment.
     */
    virtual void drawTextEx(const std::wstring& text,
                           const widget::Rect<int32_t, uint32_t>& rect,
                           const widget::Color& color,
                           const FontStyle& font,
                           TextAlign halign = TextAlign::Left,
                           VerticalAlign valign = VerticalAlign::Top) = 0;
						   
	/**
	 * @brief Draws a bitmap image.
	 * @param bitmap A pointer to the platform-specific bitmap object.
	 * @param destRect The destination rectangle to draw the bitmap in.
	 * @param opacity The opacity of the bitmap (0.0 to 1.0).
	 */
	virtual void drawBitmap(void* bitmap, 
						const widget::Rect<int32_t, uint32_t>& destRect,
						float opacity = 1.0f) = 0;

    // State management (extended)
    /** @brief Saves the current rendering state. (See `RenderContext::save`) */
    virtual void save() = 0;
    /** @brief Restores the previously saved rendering state. (See `RenderContext::restore`) */
    virtual void restore() = 0;

    /** @brief Sets the global opacity for subsequent drawing operations. */
    virtual void setOpacity(float opacity) = 0;
    /** @brief Sets the global transformation matrix. */
    virtual void setTransform(float m11, float m12, float m21, float m22,
                             float dx, float dy) = 0;
    
    /**
     * @brief Measures the width of a substring.
     * @param text The full string.
     * @param length The number of characters to measure from the beginning.
     * @param font The font style to use for measurement.
     * @return The width of the substring in device-independent pixels.
     */
    virtual float measureTextWidth(const std::wstring& text, size_t length,
                                   const FontStyle& font) const = 0;
    
    /**
     * @brief Performs hit testing on text to find a character position.
     * @param text The string to test against.
     * @param x The horizontal coordinate relative to the start of the text.
     * @param font The font style used.
     * @return The index of the character at or before the given coordinate.
     */
    virtual size_t getCharPositionFromX(const std::wstring& text, float x,
                                       const FontStyle& font) const = 0;

	/** @brief Applies a translation to the current transformation matrix. */
	virtual void translate(float dx, float dy) = 0;
};

// ============================================================================
// RENDERING HELPERS
// ============================================================================

/**
 * @brief Measures the full size of a string of text.
 * @param renderer The renderer to use for measurement.
 * @param text The text to measure.
 * @param font The font style to use.
 * @return A `Size` object containing the width and estimated height.
 */
inline widget::Size<uint32_t> measureText(
    widget::Renderer& renderer,
    const std::wstring& text,
    const FontStyle& font = FontStyle{}
) {
    // Try extended renderer first
    if (auto* extRenderer = dynamic_cast<IExtendedRenderer*>(&renderer)) {
        float width = extRenderer->measureTextWidth(text, text.length(), font);
        return widget::Size(static_cast<uint32_t>(width), 
                           static_cast<uint32_t>(font.size * 1.5f));
    }
    
    // Fallback to rough estimation
    (void)renderer; // To avoid unused parameter warning
    (void)text;
    (void)font;
    return widget::Size(100u, 20u); // Return a default placeholder size
}

/**
 * @brief Draws text centered within a given rectangle.
 * @param renderer The renderer to use.
 * @param text The text to draw.
 * @param rect The rectangle in which to center the text.
 * @param color The color of the text.
 */
inline void drawCenteredText(
    widget::Renderer& renderer,
    const std::wstring& text,
    const widget::Rect<int32_t, uint32_t>& rect,
    const widget::Color& color
) {
    auto textSize = measureText(renderer, text);
    
    int32_t x = rect.x + static_cast<int32_t>((rect.w - textSize.w) / 2);
    int32_t y = rect.y + static_cast<int32_t>((rect.h - textSize.h) / 2);
    
    widget::Rect<int32_t, uint32_t> textRect(x, y, textSize.w, textSize.h);
    renderer.drawText(text, textRect, color);
}

} // namespace frqs::render