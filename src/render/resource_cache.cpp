// src/render/resource_cache.cpp
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
    
    // Check cache
    auto it = fontCache_.find(style);
    if (it != fontCache_.end()) {
        return it->second;
    }
    
    if (!writeFactory_) {
        return nullptr;
    }
    
    // Create new text format
    IDWriteTextFormat* textFormat = nullptr;
    
    HRESULT hr = writeFactory_->CreateTextFormat(
        style.family.c_str(),
        nullptr,  // Font collection (NULL = system)
        style.bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
        style.italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        style.size,
        L"en-us",  // Locale
        &textFormat
    );
    
    if (FAILED(hr) || !textFormat) {
        return nullptr;
    }
    
    // Cache and return
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
    
    // Check cache
    auto it = brushCache_.find(key);
    if (it != brushCache_.end()) {
        return it->second;
    }
    
    // Create new brush
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
    
    // Cache and return
    brushCache_[key] = brush;
    
    return brush;
}

ID2D1Bitmap* ResourceCache::getBitmap(
    std::wstring_view path, 
    ID2D1RenderTarget* target
) {
    if (!target) {
        target = currentRenderTarget_;
    }
    
    if (!target) {
        return nullptr;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::wstring pathKey(path);  // Convert view to key
    
    // ✅ CRITICAL: Check cache first
    auto it = bitmapCache_.find(pathKey);
    if (it != bitmapCache_.end()) {
        // Increment reference count
        bitmapRefCount_[pathKey]++;
        return it->second;
    }
    
    // ✅ Cache miss: Load from disk using WIC
    ID2D1Bitmap* bitmap = loadBitmapFromWIC(path, target);
    
    if (!bitmap) {
        return nullptr;
    }
    
    // ✅ Store in cache
    bitmapCache_[pathKey] = bitmap;
    bitmapRefCount_[pathKey] = 1;
    
    return bitmap;
}

void ResourceCache::releaseBitmap(std::wstring_view path) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::wstring pathKey(path);
    
    auto refIt = bitmapRefCount_.find(pathKey);
    if (refIt == bitmapRefCount_.end()) {
        return;  // Not in cache
    }
    
    // Decrement refcount
    refIt->second--;
    
    // ✅ CRITICAL: Only release when refcount hits zero
    if (refIt->second == 0) {
        auto bitmapIt = bitmapCache_.find(pathKey);
        if (bitmapIt != bitmapCache_.end()) {
            if (bitmapIt->second) {
                bitmapIt->second->Release();
            }
            bitmapCache_.erase(bitmapIt);
        }
        bitmapRefCount_.erase(refIt);
    }
}

ID2D1Bitmap* ResourceCache::loadBitmapFromWIC(
    std::wstring_view path,
    ID2D1RenderTarget* target
) {
    if (!wicFactory_) {
        // Lazy initialize WIC factory
        HRESULT hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&wicFactory_)
        );
        
        if (FAILED(hr)) {
            return nullptr;
        }
    }
    
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;
    ID2D1Bitmap* bitmap = nullptr;
    
    std::wstring pathStr(path);  // Convert to std::wstring for API
    
    HRESULT hr = wicFactory_->CreateDecoderFromFilename(
        pathStr.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnDemand,
        &decoder
    );
    
    if (FAILED(hr)) goto cleanup_loadBitmap;
    
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) goto cleanup_loadBitmap;
    
    hr = wicFactory_->CreateFormatConverter(&converter);
    if (FAILED(hr)) goto cleanup_loadBitmap;
    
    hr = converter->Initialize(
        frame,
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeMedianCut
    );
    if (FAILED(hr)) goto cleanup_loadBitmap;
    
    hr = target->CreateBitmapFromWicBitmap(converter, nullptr, &bitmap);

cleanup_loadBitmap:
    if (converter) converter->Release();
    if (frame) frame->Release();
    if (decoder) decoder->Release();
    
    return bitmap;
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