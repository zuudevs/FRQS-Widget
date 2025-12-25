/**
 * @file image.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines the Image widget for displaying bitmap graphics.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "iwidget.hpp"
#include <string>

namespace frqs::widget {

/**
 * @class Image
 * @brief A widget that displays a bitmap image from a file.
 *
 * The Image widget can load and render common image formats (e.g., PNG, JPEG, BMP)
 * using the Windows Imaging Component (WIC). It supports various scaling modes
 * to control how the image fits within the widget's bounds.
 */
class Image : public Widget {
public:
    /**
     * @brief Defines how the image should be scaled to fit the widget's area.
     */
    enum class ScaleMode : uint8_t {
        /**
         * @brief Scales the image to fit within the widget's bounds while preserving its aspect ratio.
         * This may result in letterboxing or pillarboxing if the aspect ratios do not match.
         */
        Fit,
        /**
         * @brief Scales the image to completely fill the widget's bounds while preserving its aspect ratio.
         * This may result in parts of the image being cropped.
         */
        Fill,
        /**
         * @brief Stretches the image to fill the widget's bounds, ignoring the original aspect ratio.
         */
        Stretch
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
    /**
     * @brief Constructs a new Image widget.
     * @param path The file path to the image to load initially.
     */
    explicit Image(const std::wstring& path = L"");
    
    /**
     * @brief Destructor. Releases the underlying bitmap resource.
     */
    ~Image() override;

    /**
     * @brief Loads an image from the specified file path.
     * @param path The file path of the image to load.
     */
    void setImage(const std::wstring& path);

    /**
     * @brief Gets the current image file path.
     * @return A const reference to the image path string.
     */
    const std::wstring& getImagePath() const noexcept { return imagePath_; }

    /**
     * @brief Checks if an image has been successfully loaded.
     * @return True if an image is loaded, false otherwise.
     */
    bool hasImage() const noexcept { return bitmap_ != nullptr; }

    /**
     * @brief Sets the scaling mode for the image.
     * @param mode The desired `ScaleMode`.
     */
    void setScaleMode(ScaleMode mode) noexcept;
    
    /**
     * @brief Gets the current scaling mode.
     * @return The current `ScaleMode`.
     */
    ScaleMode getScaleMode() const noexcept { return scaleMode_; }

    /**
     * @brief Sets the opacity of the rendered image.
     * @param opacity The opacity value, from 0.0 (fully transparent) to 1.0 (fully opaque).
     */
    void setOpacity(float opacity) noexcept;
    
    /**
     * @brief Gets the current opacity of the image.
     * @return The current opacity value.
     */
    float getOpacity() const noexcept { return opacity_; }

    /**
     * @brief Renders the image using the provided renderer.
     * 
     * If the image has not been loaded into a bitmap yet, this function will
     * attempt to load it first.
     * 
     * @param renderer The renderer to draw with.
     */
    void render(Renderer& renderer) override;

private:
    void loadBitmap(Renderer& renderer);
    void releaseBitmap();
    Rect<int32_t, uint32_t> calculateDestRect() const;
};

} // namespace frqs::widget