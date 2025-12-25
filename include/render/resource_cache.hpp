/**
 * @file resource_cache.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines a singleton cache for managing graphics resources like fonts, brushes, and bitmaps.
 * @version 0.1
 * @date 2025-12-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */
 
#pragma once

#include "platform/win32_safe.hpp"
#include "unit/color.hpp"
#include <unordered_map>
#include <map>
#include <string>
#include "render/renderer.hpp"
#include <mutex>

// Forward declarations for Direct2D/DirectWrite interfaces
struct ID2D1RenderTarget;
struct ID2D1SolidColorBrush;
struct ID2D1Bitmap;
struct IDWriteFactory;
struct IDWriteTextFormat;
struct IWICImagingFactory;


namespace frqs::render {

/**
 * @struct ColorKey
 * @brief A key for caching resources based on color.
 * 
 * Provides comparison operators to be used in map-based caches.
 */
struct ColorKey {
    widget::Color color; ///< The color value.
    
    /**
     * @brief Constructs a ColorKey from a `widget::Color`.
     * @param c The color.
     */
    explicit ColorKey(const widget::Color& c) : color(c) {}

    /**
     * @brief Equality comparison operator.
     * @param other The other ColorKey to compare.
     * @return `true` if colors are identical, `false` otherwise.
     */
    bool operator==(const ColorKey& other) const noexcept {
        return color.r == other.color.r && color.g == other.color.g &&
               color.b == other.color.b && color.a == other.color.a;
    }

    /**
     * @brief Less-than comparison operator for ordering in maps.
     * @param other The other ColorKey to compare.
     * @return `true` if this color should be ordered before the other.
     */
    bool operator<(const ColorKey& other) const noexcept {
        if (color.r != other.color.r) return color.r < other.color.r;
        if (color.g != other.color.g) return color.g < other.color.g;
        if (color.b != other.color.b) return color.b < other.color.b;
        return color.a < other.color.a;
    }
};

} // namespace frqs::render

namespace std {
    /**
     * @brief `std::hash` specialization for `frqs::render::FontStyle`.
     * 
     * Allows `FontStyle` to be used as a key in `std::unordered_map`.
     */
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

/**
 * @class ResourceCache
 * @brief A singleton class for caching and managing graphics resources.
 * 
 * This class reduces resource creation overhead by caching objects like
 * DirectWrite text formats, Direct2D brushes, and bitmaps. It is designed to be
 * thread-safe.
 */
class ResourceCache {
private:
    mutable std::mutex mutex_; ///< Mutex for thread-safe access to caches.
    
    IDWriteFactory* writeFactory_ = nullptr; ///< The DirectWrite factory.
    IWICImagingFactory* wicFactory_ = nullptr; ///< The WIC factory for image loading.
    
    std::unordered_map<FontStyle, IDWriteTextFormat*> fontCache_; ///< Cache for text formats.
    std::map<ColorKey, ID2D1SolidColorBrush*> brushCache_; ///< Cache for solid color brushes.
	std::unordered_map<std::wstring, ID2D1Bitmap*> bitmapCache_; ///< Cache for bitmaps, keyed by file path.
    std::unordered_map<std::wstring, size_t> bitmapRefCount_; ///< Reference counting for bitmaps.
    
    ID2D1RenderTarget* currentRenderTarget_ = nullptr; ///< The current Direct2D render target.
    
    /**
     * @brief Private constructor to enforce singleton pattern.
     * 
     * Initializes DirectWrite and WIC factories.
     */
    ResourceCache();

    /**
     * @brief Loads a bitmap from a file using WIC.
     * @param path The file path of the image.
     * @param target The render target to create the bitmap for.
     * @return A pointer to the `ID2D1Bitmap`, or `nullptr` on failure.
     */
	ID2D1Bitmap* loadBitmapFromWIC(
		std::wstring_view path,
		ID2D1RenderTarget* target
	);
    
public:
    /**
     * @brief Destructor. Releases all cached resources.
     */
    ~ResourceCache() noexcept;
    
    /**
     * @brief Gets the singleton instance of the ResourceCache.
     * @return A reference to the singleton instance.
     */
    static ResourceCache& instance() noexcept {
        static ResourceCache cache;
        return cache;
    }
    
    // Delete copy/move constructors and assignments.
    ResourceCache(const ResourceCache&) = delete;
    ResourceCache& operator=(const ResourceCache&) = delete;
    ResourceCache(ResourceCache&&) = delete;
    ResourceCache& operator=(ResourceCache&&) = delete;
    
    /**
     * @brief Retrieves a cached or creates a new `IDWriteTextFormat`.
     * @param style The `FontStyle` describing the desired format.
     * @return A pointer to the `IDWriteTextFormat`.
     */
    IDWriteTextFormat* getFont(const FontStyle& style);

    /**
     * @brief Retrieves a cached or creates a new `ID2D1SolidColorBrush`.
     * @param color The desired color of the brush.
     * @param target The render target to create the brush for. If `nullptr`, uses the current target.
     * @return A pointer to the `ID2D1SolidColorBrush`.
     */
    ID2D1SolidColorBrush* getBrush(const widget::Color& color, ID2D1RenderTarget* target = nullptr);

    /**
     * @brief Retrieves a cached or loads a new `ID2D1Bitmap`.
     * @param path The file path of the bitmap.
     * @param target The render target to create the bitmap for.
     * @return A pointer to the `ID2D1Bitmap`.
     */
	ID2D1Bitmap* getBitmap(std::wstring_view path, ID2D1RenderTarget* target);
    
    /**
     * @brief Sets the current render target.
     * Brushes and other target-dependent resources will be created for this target.
     * @param target The active `ID2D1RenderTarget`.
     */
    void setRenderTarget(ID2D1RenderTarget* target) noexcept {
        currentRenderTarget_ = target;
    }
    
    /** @brief Clears all cached brushes. Called when the render target changes. */
    void clearBrushCache();
    /** @brief Clears all cached font formats. */
    void clearFontCache();
    /** @brief Clears all cached resources. */
    void clearAll();

    /**
     * @brief Decrements the reference count for a bitmap and releases it if the count reaches zero.
     * @param path The file path of the bitmap to release.
     */
	void releaseBitmap(std::wstring_view path);
    
    /**
     * @brief Gets the DirectWrite factory.
     * @return A pointer to the `IDWriteFactory`.
     */
    IDWriteFactory* getWriteFactory() const noexcept {
        return writeFactory_;
    }
};

} // namespace frqs::render