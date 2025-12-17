#pragma once

// Note: The Renderer interface is already defined in widget/iwidget.hpp
// This header provides additional rendering utilities

#include "../widget/iwidget.hpp"
#include <numbers>

namespace frqs::render {

// ============================================================================
// RENDERING UTILITIES
// ============================================================================

// Helper to convert Color to normalized float [0-1]
struct ColorF {
    float r, g, b, a;

    constexpr ColorF(const widget::Color& color) noexcept
        : r(color.r / 255.0f)
        , g(color.g / 255.0f)
        , b(color.b / 255.0f)
        , a(color.a / 255.0f) {}

    constexpr ColorF(float r, float g, float b, float a = 1.0f) noexcept
        : r(r), g(g), b(b), a(a) {}
};

// ============================================================================
// TEXT ALIGNMENT
// ============================================================================

enum class TextAlign : uint8_t {
    Left,
    Center,
    Right,
    Justify
};

enum class VerticalAlign : uint8_t {
    Top,
    Middle,
    Bottom
};

// ============================================================================
// FONT STYLE
// ============================================================================

struct FontStyle {
    std::wstring family = L"Segoe UI";
    float size = 14.0f;
    bool bold = false;
    bool italic = false;
    bool underline = false;
    bool strikethrough = false;
};

// ============================================================================
// BRUSH TYPES
// ============================================================================

enum class BrushType : uint8_t {
    Solid,
    LinearGradient,
    RadialGradient,
    Bitmap
};

// ============================================================================
// EXTENDED RENDERER INTERFACE (Optional)
// ============================================================================

class IExtendedRenderer : public widget::Renderer {
public:
    // Advanced drawing
    virtual void drawLine(const widget::Point<int32_t>& start,
                         const widget::Point<int32_t>& end,
                         const widget::Color& color,
                         float strokeWidth = 1.0f) = 0;

    virtual void drawEllipse(const widget::Rect<int32_t, uint32_t>& rect,
                            const widget::Color& color,
                            float strokeWidth = 1.0f) = 0;

    virtual void fillEllipse(const widget::Rect<int32_t, uint32_t>& rect,
                            const widget::Color& color) = 0;

    virtual void drawRoundedRect(const widget::Rect<int32_t, uint32_t>& rect,
                                float radiusX, float radiusY,
                                const widget::Color& color,
                                float strokeWidth = 1.0f) = 0;

    virtual void fillRoundedRect(const widget::Rect<int32_t, uint32_t>& rect,
                                float radiusX, float radiusY,
                                const widget::Color& color) = 0;

    // Text with advanced options
    virtual void drawTextEx(const std::wstring& text,
                           const widget::Rect<int32_t, uint32_t>& rect,
                           const widget::Color& color,
                           const FontStyle& font,
                           TextAlign halign = TextAlign::Left,
                           VerticalAlign valign = VerticalAlign::Top) = 0;

    // State management (extended)
    virtual void save() = 0;
    virtual void restore() = 0;

    virtual void setOpacity(float opacity) = 0;
    virtual void setTransform(float m11, float m12, float m21, float m22,
                             float dx, float dy) = 0;
    
    // ========================================================================
    // TEXT MEASUREMENT (NEW!)
    // ========================================================================
    
    // Measure text width up to a specific character position
    virtual float measureTextWidth(const std::wstring& text, size_t length,
                                   const FontStyle& font) const = 0;
    
    // Get character position from X coordinate (hit testing)
    virtual size_t getCharPositionFromX(const std::wstring& text, float x,
                                       const FontStyle& font) const = 0;
};

// ============================================================================
// RENDERING HELPERS
// ============================================================================

// Calculate text size (requires renderer implementation)
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
    (void)text;
    (void)font;
    return widget::Size(100u, 20u);
}

// Draw centered text
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