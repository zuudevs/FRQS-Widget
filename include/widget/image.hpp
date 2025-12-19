// include/widget/image.hpp - FIXED VERSION
#pragma once

#include "iwidget.hpp"
#include <string>

namespace frqs::widget {

// ============================================================================
// IMAGE WIDGET (Displays bitmap images via WIC)
// ============================================================================

class Image : public Widget {
public:
    enum class ScaleMode : uint8_t {
        Fit,     // Scale to fit, preserve aspect ratio (letterbox/pillarbox)
        Fill,    // Scale to fill, preserve aspect ratio (may crop)
        Stretch  // Stretch to fill, ignore aspect ratio
    };

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;

    // Image data
    void* bitmap_ = nullptr;  // ID2D1Bitmap* (opaque pointer)
    std::wstring imagePath_;
    ScaleMode scaleMode_ = ScaleMode::Fit;
    
    // Background (for transparency/letterboxing)
    Color backgroundColor_ = colors::Transparent;
    
    // Opacity
    float opacity_ = 1.0f;

public:
    explicit Image(const std::wstring& path = L"");
    ~Image() override;

    // Image loading
    void setImage(const std::wstring& path);
    const std::wstring& getImagePath() const noexcept { return imagePath_; }
    bool hasImage() const noexcept { return bitmap_ != nullptr; }

    // Scale mode
    void setScaleMode(ScaleMode mode) noexcept;
    ScaleMode getScaleMode() const noexcept { return scaleMode_; }

    // Visual properties
    void setOpacity(float opacity) noexcept;
    float getOpacity() const noexcept { return opacity_; }

    // Rendering
    void render(Renderer& renderer) override;

private:
    void loadBitmap(Renderer& renderer);
    void releaseBitmap();
    Rect<int32_t, uint32_t> calculateDestRect() const;
};

} // namespace frqs::widget