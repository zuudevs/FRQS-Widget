// src/widget/image.cpp - FIXED VERSION
#include "widget/image.hpp"
#include "render/renderer.hpp"
#include "../render/renderer_d2d.hpp"  // Full header for dynamic_cast

namespace frqs::widget {

// ============================================================================
// IMAGE PIMPL
// ============================================================================

struct Image::Impl {
    Size<uint32_t> bitmapSize{0, 0};  // Original bitmap dimensions
    bool loadAttempted = false;        // Prevent repeated load failures
};

// ============================================================================
// IMAGE IMPLEMENTATION
// ============================================================================

Image::Image(const std::wstring& path)
    : Widget()
    , pImpl_(std::make_unique<Impl>())
    , imagePath_(path)
{
    setBackgroundColor(colors::Transparent);
}

Image::~Image() {
    releaseBitmap();
}

void Image::setImage(const std::wstring& path) {
    if (imagePath_ == path) return;
    
    releaseBitmap();
    imagePath_ = path;
    pImpl_->loadAttempted = false;
    invalidate();
}

void Image::setScaleMode(ScaleMode mode) noexcept {
    if (scaleMode_ == mode) return;
    scaleMode_ = mode;
    invalidate();
}

void Image::setOpacity(float opacity) noexcept {
    opacity_ = std::clamp(opacity, 0.0f, 1.0f);
    invalidate();
}

void Image::loadBitmap(Renderer& renderer) {
    // Don't retry failed loads
    if (pImpl_->loadAttempted) return;
    pImpl_->loadAttempted = true;
    
    if (imagePath_.empty()) return;
    
    // Try to use extended renderer for bitmap loading
    auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer);
    if (!extRenderer) return;
    
    // Cast to RendererD2D to access loadBitmapFromFile
    auto* d2dRenderer = dynamic_cast<render::RendererD2D*>(extRenderer);
    if (!d2dRenderer) return;
    
    // Load bitmap
    auto* d2dBitmap = d2dRenderer->loadBitmapFromFile(imagePath_);
    if (!d2dBitmap) return;
    
    bitmap_ = d2dBitmap;
    
    // Get bitmap size
    auto size = d2dBitmap->GetSize();
    pImpl_->bitmapSize = Size<uint32_t>(
        static_cast<uint32_t>(size.width),
        static_cast<uint32_t>(size.height)
    );
}

void Image::releaseBitmap() {
    if (bitmap_) {
        auto* d2dBitmap = static_cast<ID2D1Bitmap*>(bitmap_);
        d2dBitmap->Release();
        bitmap_ = nullptr;
        pImpl_->bitmapSize = Size<uint32_t>(0, 0);
    }
}

Rect<int32_t, uint32_t> Image::calculateDestRect() const {
    auto widgetRect = getRect();
    
    if (!bitmap_ || pImpl_->bitmapSize.w == 0 || pImpl_->bitmapSize.h == 0) {
        return widgetRect;
    }
    
    float widgetAspect = static_cast<float>(widgetRect.w) / widgetRect.h;
    float bitmapAspect = static_cast<float>(pImpl_->bitmapSize.w) / pImpl_->bitmapSize.h;
    
    switch (scaleMode_) {
        case ScaleMode::Stretch:
            // Fill entire widget rect, ignore aspect ratio
            return widgetRect;
            
        case ScaleMode::Fill: {
            // Scale to fill, preserve aspect ratio, may crop
            uint32_t destW, destH;
            
            if (widgetAspect > bitmapAspect) {
                // Widget is wider - fit width, crop height
                destW = widgetRect.w;
                destH = static_cast<uint32_t>(widgetRect.w / bitmapAspect);
            } else {
                // Widget is taller - fit height, crop width
                destH = widgetRect.h;
                destW = static_cast<uint32_t>(widgetRect.h * bitmapAspect);
            }
            
            // Center the image
            int32_t destX = widgetRect.x + static_cast<int32_t>((widgetRect.w - destW) / 2);
            int32_t destY = widgetRect.y + static_cast<int32_t>((widgetRect.h - destH) / 2);
            
            return Rect<int32_t, uint32_t>(destX, destY, destW, destH);
        }
        
        case ScaleMode::Fit:
        default: {
            // Scale to fit, preserve aspect ratio, may letterbox
            uint32_t destW, destH;
            
            if (widgetAspect > bitmapAspect) {
                // Widget is wider - fit height, add pillarbox
                destH = widgetRect.h;
                destW = static_cast<uint32_t>(widgetRect.h * bitmapAspect);
            } else {
                // Widget is taller - fit width, add letterbox
                destW = widgetRect.w;
                destH = static_cast<uint32_t>(widgetRect.w / bitmapAspect);
            }
            
            // Center the image
            int32_t destX = widgetRect.x + static_cast<int32_t>((widgetRect.w - destW) / 2);
            int32_t destY = widgetRect.y + static_cast<int32_t>((widgetRect.h - destH) / 2);
            
            return Rect<int32_t, uint32_t>(destX, destY, destW, destH);
        }
    }
}

void Image::render(Renderer& renderer) {
    if (!isVisible()) return;

    auto rect = getRect();
    
    // Render background
    Widget::render(renderer);
    
    // Load bitmap if needed
    if (!bitmap_ && !pImpl_->loadAttempted) {
        loadBitmap(renderer);
    }
    
    // Draw bitmap if available
    if (bitmap_) {
        auto destRect = calculateDestRect();
        
        // Try to use extended renderer
        if (auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer)) {
            extRenderer->drawBitmap(bitmap_, destRect, opacity_);
        }
    }
}

} // namespace frqs::widget