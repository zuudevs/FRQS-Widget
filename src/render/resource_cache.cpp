#include "render/resource_cache.hpp"
#include <stdexcept>

namespace frqs::render {

// ============================================================================
// CONSTRUCTOR
// ============================================================================

ResourceCache::ResourceCache() {
    HRESULT hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&writeFactory_)
    );
    
    if (FAILED(hr)) {
        throw std::runtime_error("ResourceCache: Failed to create DirectWrite factory");
    }
}

// ============================================================================
// DESTRUCTOR
// ============================================================================

ResourceCache::~ResourceCache() noexcept {
    clearAll();
    
    if (writeFactory_) {
        writeFactory_->Release();
        writeFactory_ = nullptr;
    }
}

// ============================================================================
// GET FONT
// ============================================================================

IDWriteTextFormat* ResourceCache::getFont(const FontStyle& style) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = fontCache_.find(style);
    if (it != fontCache_.end()) {
        return it->second;
    }
    
    if (!writeFactory_) {
        return nullptr;
    }
    
    IDWriteTextFormat* textFormat = nullptr;
    
    HRESULT hr = writeFactory_->CreateTextFormat(
        style.family.c_str(),
        nullptr,
        style.bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
        style.italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        style.size,
        L"en-us",
        &textFormat
    );
    
    if (FAILED(hr) || !textFormat) {
        return nullptr;
    }
    
    fontCache_[style] = textFormat;
    
    return textFormat;
}

// ============================================================================
// GET BRUSH
// ============================================================================

ID2D1SolidColorBrush* ResourceCache::getBrush(
    const widget::Color& color,
    ID2D1RenderTarget* target
) {
    if (!target) {
        target = currentRenderTarget_;
    }
    
    if (!target) {
        return nullptr;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    ColorKey key(color);
    
    auto it = brushCache_.find(key);
    if (it != brushCache_.end()) {
        return it->second;
    }
    
    ID2D1SolidColorBrush* brush = nullptr;
    
    D2D1_COLOR_F d2dColor = D2D1::ColorF(
        color.r / 255.0f,
        color.g / 255.0f,
        color.b / 255.0f,
        color.a / 255.0f
    );
    
    HRESULT hr = target->CreateSolidColorBrush(d2dColor, &brush);
    
    if (FAILED(hr) || !brush) {
        return nullptr;
    }
    
    brushCache_[key] = brush;
    
    return brush;
}

// ============================================================================
// CLEAR CACHES
// ============================================================================

void ResourceCache::clearBrushCache() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& [key, brush] : brushCache_) {
        if (brush) {
            brush->Release();
        }
    }
    
    brushCache_.clear();
}

void ResourceCache::clearFontCache() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& [style, format] : fontCache_) {
        if (format) {
            format->Release();
        }
    }
    
    fontCache_.clear();
}

void ResourceCache::clearAll() {
    clearBrushCache();
    clearFontCache();
}

} // namespace frqs::render