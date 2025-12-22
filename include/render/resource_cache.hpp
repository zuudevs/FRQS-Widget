#pragma once

#include "platform/win32_safe.hpp"
#include "unit/color.hpp"
#include <unordered_map>
#include <string>
#include "render/renderer.hpp"
#include <mutex>

// ============================================================================
// HASH SPECIALIZATION for FontStyle
// ============================================================================

namespace std {
    template<>
    struct hash<frqs::render::FontStyle> {
        size_t operator()(const frqs::render::FontStyle& fs) const noexcept {
            size_t h1 = hash<wstring>{}(fs.family);
            size_t h2 = hash<float>{}(fs.size);
            size_t h3 = hash<bool>{}(fs.bold);
            size_t h4 = hash<bool>{}(fs.italic);
            size_t h5 = hash<bool>{}(fs.underline);
            size_t h6 = hash<bool>{}(fs.strikethrough);
            
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4) ^ (h6 << 5);
        }
    };
}

namespace frqs::render {

// ============================================================================
// COLOR KEY (for brush cache)
// ============================================================================

struct ColorKey {
    widget::Color color;
    
    explicit ColorKey(const widget::Color& c) : color(c) {}
    
    bool operator==(const ColorKey& other) const noexcept {
        return color.r == other.color.r &&
               color.g == other.color.g &&
               color.b == other.color.b &&
               color.a == other.color.a;
    }
};

} // namespace frqs::render

// ============================================================================
// HASH SPECIALIZATION for ColorKey
// ============================================================================

namespace std {
    template<>
    struct hash<frqs::render::ColorKey> {
        size_t operator()(const frqs::render::ColorKey& ck) const noexcept {
            return (static_cast<size_t>(ck.color.r) << 24) |
                   (static_cast<size_t>(ck.color.g) << 16) |
                   (static_cast<size_t>(ck.color.b) << 8) |
                   static_cast<size_t>(ck.color.a);
        }
    };
}

namespace frqs::render {

// ============================================================================
// RESOURCE CACHE (Singleton)
// ============================================================================

class ResourceCache {
private:
    mutable std::mutex mutex_;
    
    IDWriteFactory* writeFactory_ = nullptr;
    
    std::unordered_map<FontStyle, IDWriteTextFormat*> fontCache_;
    std::unordered_map<ColorKey, ID2D1SolidColorBrush*> brushCache_;
    
    ID2D1RenderTarget* currentRenderTarget_ = nullptr;
    
    ResourceCache();
    
public:
    ~ResourceCache() noexcept;
    
    static ResourceCache& instance() noexcept {
        static ResourceCache cache;
        return cache;
    }
    
    ResourceCache(const ResourceCache&) = delete;
    ResourceCache& operator=(const ResourceCache&) = delete;
    ResourceCache(ResourceCache&&) = delete;
    ResourceCache& operator=(ResourceCache&&) = delete;
    
    IDWriteTextFormat* getFont(const FontStyle& style);
    
    ID2D1SolidColorBrush* getBrush(const widget::Color& color, ID2D1RenderTarget* target);
    
    void setRenderTarget(ID2D1RenderTarget* target) noexcept {
        currentRenderTarget_ = target;
    }
    
    void clearBrushCache();
    
    void clearFontCache();
    
    void clearAll();
};

} // namespace frqs::render