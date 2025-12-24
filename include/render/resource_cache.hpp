// include/render/resource_cache.hpp
#pragma once

#include "platform/win32_safe.hpp"
#include "unit/color.hpp"
#include <unordered_map>
#include <map>
#include <string>
#include "render/renderer.hpp"
#include <mutex>

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
    
    bool operator<(const ColorKey& other) const noexcept {
        if (color.r != other.color.r) return color.r < other.color.r;
        if (color.g != other.color.g) return color.g < other.color.g;
        if (color.b != other.color.b) return color.b < other.color.b;
        return color.a < other.color.a;
    }
};

} // namespace frqs::render

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
// RESOURCE CACHE (Singleton)
// ============================================================================

class ResourceCache {
private:
    mutable std::mutex mutex_;
    
    IDWriteFactory* writeFactory_ = nullptr;
    
    std::unordered_map<FontStyle, IDWriteTextFormat*> fontCache_;
    std::map<ColorKey, ID2D1SolidColorBrush*> brushCache_;
	std::unordered_map<std::wstring, ID2D1Bitmap*> bitmapCache_;
    std::unordered_map<std::wstring, size_t> bitmapRefCount_;
    
    ID2D1RenderTarget* currentRenderTarget_ = nullptr;
    
    ResourceCache();
	ID2D1Bitmap* loadBitmapFromWIC(
		std::wstring_view path,
		ID2D1RenderTarget* target
	);
    
public:
    ~ResourceCache() noexcept;
    
    // Singleton access
    static ResourceCache& instance() noexcept {
        static ResourceCache cache;
        return cache;
    }
    
    // Non-copyable, non-movable
    ResourceCache(const ResourceCache&) = delete;
    ResourceCache& operator=(const ResourceCache&) = delete;
    ResourceCache(ResourceCache&&) = delete;
    ResourceCache& operator=(ResourceCache&&) = delete;
    
    // Font management
    IDWriteTextFormat* getFont(const FontStyle& style);
    
    // Brush management
    ID2D1SolidColorBrush* getBrush(const widget::Color& color, ID2D1RenderTarget* target = nullptr);

	ID2D1Bitmap* getBitmap(std::wstring_view path, ID2D1RenderTarget* target);
    
    // Render target management
    void setRenderTarget(ID2D1RenderTarget* target) noexcept {
        currentRenderTarget_ = target;
    }
    
    // Cache management
    void clearBrushCache();
    void clearFontCache();
    void clearAll();

	void releaseBitmap(std::wstring_view path);
    
    // DirectWrite factory access
    IDWriteFactory* getWriteFactory() const noexcept {
        return writeFactory_;
    }
};

} // namespace frqs::render