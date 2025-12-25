/**
 * @file image.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Implementation of the Image widget.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "widget/image.hpp"
#include "render/renderer.hpp"
#include "render/renderer_d2d.hpp"  // Full header for dynamic_cast

namespace frqs::widget {

// ============================================================================
// IMAGE PIMPL
// ============================================================================

/**
 * @brief Private implementation details for the Image widget.
 */
struct Image::Impl {
    Size<uint32_t> bitmapSize{0, 0};  ///< Original bitmap dimensions.
    bool loadAttempted = false;        ///< Prevents repeated load failures.
};

// ============================================================================
// IMAGE IMPLEMENTATION
// ============================================================================

/**
 * @brief Constructs an Image widget.
 * @param path The file path to the image to display.
 */
Image::Image(const std::wstring& path)
    : Widget()
    , pImpl_(std::make_unique<Impl>())
    , imagePath_(path)
{
    setBackgroundColor(colors::Transparent);
}

/**
 * @brief Destroys the Image widget, releasing any bitmap resources.
 */
Image::~Image() {
    releaseBitmap();
}

/**
 * @brief Sets or changes the image displayed by the widget.
 * @param path The file path to the new image.
 */
void Image::setImage(const std::wstring& path) {
    if (imagePath_ == path) return;
    
    releaseBitmap();
    imagePath_ = path;
    pImpl_->loadAttempted = false;
    invalidate();
}

/**
 * @brief Sets the scaling mode for the image within the widget's bounds.
 * @param mode The new ScaleMode to use.
 */
void Image::setScaleMode(ScaleMode mode) noexcept {
    if (scaleMode_ == mode) return;
    scaleMode_ = mode;
    invalidate();
}

/**
 * @brief Sets the opacity of the image.
 * @param opacity The opacity level, from 0.0 (transparent) to 1.0 (opaque).
 */
void Image::setOpacity(float opacity) noexcept {
    opacity_ = std::clamp(opacity, 0.0f, 1.0f);
    invalidate();
}

/**
 * @brief Loads the bitmap from the specified path using the given renderer.
 * @details This is an internal method called by the rendering pipeline.
 * It attempts to load the bitmap resource only once.
 * @param renderer The renderer to use for loading the bitmap.
 */
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

/**
 * @brief Releases the currently held bitmap resource.
 */
void Image::releaseBitmap() {
    if (bitmap_) {
        auto* d2dBitmap = static_cast<ID2D1Bitmap*>(bitmap_);
        d2dBitmap->Release();
        bitmap_ = nullptr;
        pImpl_->bitmapSize = Size<uint32_t>(0, 0);
    }
}

/**
 * @brief Calculates the destination rectangle for drawing the bitmap based on the current scale mode.
 * @return The calculated rectangle within the widget's bounds where the image should be drawn.
 */
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

/**
 * @brief Renders the image widget.
 * @details This method is called by the rendering pipeline. It handles drawing the widget's background,
 * loading the bitmap if necessary, and drawing the bitmap according to the scale mode and opacity.
 * @param renderer The renderer to use for drawing.
 */
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