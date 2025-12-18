#pragma once

#include "../../include/render/renderer.hpp"
#include "../../include/platform/win32_safe.hpp"
#include <stack>

namespace frqs::render {

// ============================================================================
// DIRECT2D RENDERER IMPLEMENTATION (Private)
// ============================================================================

class RendererD2D : public IExtendedRenderer {
private:
    // Direct2D resources
    ID2D1Factory* factory_ = nullptr;
    ID2D1HwndRenderTarget* renderTarget_ = nullptr;
    IDWriteFactory* writeFactory_ = nullptr;
    IDWriteTextFormat* defaultTextFormat_ = nullptr;
	IWICImagingFactory* wicFactory_ = nullptr;
    
    // State
    platform::NativeHandle hwnd_;
    std::stack<widget::Rect<int32_t, uint32_t>> clipStack_;
	std::stack<D2D1_MATRIX_3X2_F> transformStack_;
    bool inRender_ = false;

public:
    explicit RendererD2D(platform::NativeHandle hwnd);
    ~RendererD2D() noexcept override;

    // Non-copyable, non-movable
    RendererD2D(const RendererD2D&) = delete;
    RendererD2D& operator=(const RendererD2D&) = delete;

    // ========================================================================
    // RENDER SESSION
    // ========================================================================

    void beginRender();
    void endRender();
    bool isRendering() const noexcept { return inRender_; }

    // ========================================================================
    // BASIC RENDERER INTERFACE
    // ========================================================================

    void clear(const widget::Color& color) override;
    
    void drawRect(const widget::Rect<int32_t, uint32_t>& rect,
                 const widget::Color& color,
                 float strokeWidth = 1.0f) override;
    
    void fillRect(const widget::Rect<int32_t, uint32_t>& rect,
                 const widget::Color& color) override;
    
    void drawText(const std::wstring& text,
                 const widget::Rect<int32_t, uint32_t>& rect,
                 const widget::Color& color) override;

    void pushClip(const widget::Rect<int32_t, uint32_t>& rect) override;
    void popClip() override;

    // ========================================================================
    // EXTENDED RENDERER INTERFACE
    // ========================================================================

    void drawLine(const widget::Point<int32_t>& start,
                 const widget::Point<int32_t>& end,
                 const widget::Color& color,
                 float strokeWidth = 1.0f) override;

    void drawEllipse(const widget::Rect<int32_t, uint32_t>& rect,
                    const widget::Color& color,
                    float strokeWidth = 1.0f) override;

    void fillEllipse(const widget::Rect<int32_t, uint32_t>& rect,
                    const widget::Color& color) override;

    void drawRoundedRect(const widget::Rect<int32_t, uint32_t>& rect,
                        float radiusX, float radiusY,
                        const widget::Color& color,
                        float strokeWidth = 1.0f) override;

    void fillRoundedRect(const widget::Rect<int32_t, uint32_t>& rect,
                        float radiusX, float radiusY,
                        const widget::Color& color) override;

    void drawTextEx(const std::wstring& text,
                   const widget::Rect<int32_t, uint32_t>& rect,
                   const widget::Color& color,
                   const FontStyle& font,
                   TextAlign halign = TextAlign::Left,
                   VerticalAlign valign = VerticalAlign::Top) override;

    void save() override;
    void restore() override;

    void setOpacity(float opacity) override;
    void setTransform(float m11, float m12, float m21, float m22,
                     float dx, float dy) override;

	void translate(float dx, float dy) override;

    // ========================================================================
    // TEXT MEASUREMENT (NEW!)
    // ========================================================================
    
    // Measure text width up to a specific position
    float measureTextWidth(const std::wstring& text, size_t length, 
                          const FontStyle& font) const;
    
    // Get character position from X coordinate (hit testing)
    size_t getCharPositionFromX(const std::wstring& text, float x,
                                const FontStyle& font) const;

    // ========================================================================
    // RESOURCE MANAGEMENT
    // ========================================================================

    void resize(uint32_t width, uint32_t height);
    bool recreateDeviceResources();

private:
    // Helper methods
    ID2D1SolidColorBrush* createSolidBrush(const widget::Color& color);
    D2D1_COLOR_F toD2DColor(const widget::Color& color) const noexcept;
    D2D1_RECT_F toD2DRect(const widget::Rect<int32_t, uint32_t>& rect) const noexcept;
    
    void cleanup() noexcept;
    void cleanupDeviceResources() noexcept;
};

} // namespace frqs::render