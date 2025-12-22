#pragma once
#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>
#include <unordered_map>
#include <string>
#include <expected>
#include <span>

namespace frqs::render {

using Microsoft::WRL::ComPtr;

// ============================================================================
// Brush Cache - Prevents recreating solid color brushes
// ============================================================================
struct BrushKey {
    float r, g, b, a;
    
    bool operator==(const BrushKey& o) const noexcept {
        return r == o.r && g == o.g && b == o.b && a == o.a;
    }
};

struct BrushKeyHash {
    size_t operator()(const BrushKey& k) const noexcept {
        size_t h = 0;
        h ^= std::hash<float>{}(k.r) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<float>{}(k.g) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<float>{}(k.b) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<float>{}(k.a) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

class BrushCache {
public:
    explicit BrushCache(ID2D1RenderTarget* rt) : render_target_(rt) {}
    
    // Returns a cached brush or creates one
    ID2D1SolidColorBrush* get_or_create(float r, float g, float b, float a = 1.0f) {
        BrushKey key{r, g, b, a};
        
        if (auto it = cache_.find(key); it != cache_.end()) {
            return it->second.Get();
        }
        
        ComPtr<ID2D1SolidColorBrush> brush;
        D2D1_COLOR_F color{r, g, b, a};
        
        if (FAILED(render_target_->CreateSolidColorBrush(color, &brush))) {
            return nullptr;
        }
        
        auto* ptr = brush.Get();
        cache_[key] = std::move(brush);
        return ptr;
    }
    
    void clear() { cache_.clear(); }
    size_t size() const noexcept { return cache_.size(); }
    
private:
    ID2D1RenderTarget* render_target_;
    std::unordered_map<BrushKey, ComPtr<ID2D1SolidColorBrush>, BrushKeyHash> cache_;
};

// ============================================================================
// Font Cache - Reuses text formats
// ============================================================================
struct FontKey {
    std::wstring family;
    float size;
    DWRITE_FONT_WEIGHT weight;
    DWRITE_FONT_STYLE style;
    
    bool operator==(const FontKey& o) const noexcept {
        return family == o.family && size == o.size && 
               weight == o.weight && style == o.style;
    }
};

struct FontKeyHash {
    size_t operator()(const FontKey& k) const noexcept {
        size_t h = std::hash<std::wstring>{}(k.family);
        h ^= std::hash<float>{}(k.size) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<int>{}(k.weight) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<int>{}(k.style) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

class FontCache {
public:
    explicit FontCache(IDWriteFactory* factory) : dwrite_factory_(factory) {}
    
    IDWriteTextFormat* get_or_create(
        std::wstring_view family,
        float size,
        DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL
    ) {
        FontKey key{std::wstring(family), size, weight, style};
        
        if (auto it = cache_.find(key); it != cache_.end()) {
            return it->second.Get();
        }
        
        ComPtr<IDWriteTextFormat> format;
        
        HRESULT hr = dwrite_factory_->CreateTextFormat(
            key.family.c_str(),
            nullptr,
            weight,
            style,
            DWRITE_FONT_STRETCH_NORMAL,
            size,
            L"en-us",
            &format
        );
        
        if (FAILED(hr)) {
            return nullptr;
        }
        
        auto* ptr = format.Get();
        cache_[key] = std::move(format);
        return ptr;
    }
    
    void clear() { cache_.clear(); }
    size_t size() const noexcept { return cache_.size(); }
    
private:
    IDWriteFactory* dwrite_factory_;
    std::unordered_map<FontKey, ComPtr<IDWriteTextFormat>, FontKeyHash> cache_;
};

// ============================================================================
// Unified Resource Manager
// ============================================================================
class ResourceCache {
public:
    ResourceCache(ID2D1RenderTarget* rt, IDWriteFactory* dwrite)
        : brush_cache_(rt), font_cache_(dwrite) {}
    
    BrushCache& brushes() noexcept { return brush_cache_; }
    FontCache& fonts() noexcept { return font_cache_; }
    
    void clear_all() {
        brush_cache_.clear();
        font_cache_.clear();
    }
    
    struct Stats {
        size_t brush_count;
        size_t font_count;
    };
    
    Stats get_stats() const noexcept {
        return {brush_cache_.size(), font_cache_.size()};
    }
    
private:
    BrushCache brush_cache_;
    FontCache font_cache_;
};

} // namespace frqs::render