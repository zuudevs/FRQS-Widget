/**
 * @file slider.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines the Slider widget, a control for selecting a numeric value from a range.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "iwidget.hpp"
#include <functional>
#include <cmath>
#include "render/renderer.hpp"

namespace frqs::widget {

// ============================================================================
// SLIDER WIDGET (Horizontal/Vertical numeric slider)
// ============================================================================

/**
 * @brief A widget that allows a user to select a numeric value by dragging a handle (thumb) along a track.
 * @details The slider can be oriented horizontally or vertically. It supports a configurable
 * minimum/maximum range and can be set to move in discrete steps or continuously.
 */
class Slider : public Widget {
public:
    /**
     * @brief A callback function type invoked when the slider's value changes.
     * @param value The new value of the slider.
     */
    using ValueChangedCallback = std::function<void(double)>;

    /**
     * @brief Defines the orientation of the slider.
     */
    enum class Orientation : uint8_t {
        Horizontal, ///< The slider is laid out horizontally.
        Vertical    ///< The slider is laid out vertically.
    };

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;

    // Value
    double value_ = 0.0;        ///< The current value of the slider.
    double minValue_ = 0.0;     ///< The minimum value of the slider's range.
    double maxValue_ = 100.0;   ///< The maximum value of the slider's range.
    double step_ = 1.0;         ///< The increment for snapping. If 0, the slider is continuous.
    
    // State
    Orientation orientation_ = Orientation::Horizontal; ///< The slider's orientation.
    bool dragging_ = false;     ///< True if the user is currently dragging the thumb.
    bool hovered_ = false;      ///< True if the mouse is currently hovering over the thumb.
    bool enabled_ = true;       ///< If false, the slider is non-interactive and appears grayed out.
    
    // Styling
    Color trackColor_ = Color(189, 195, 199);        ///< The color of the slider's background track.
    Color fillColor_ = Color(52, 152, 219);         ///< The color of the track portion representing the current value.
    Color thumbColor_ = colors::White;              ///< The main color of the draggable thumb.
    Color thumbHoverColor_ = Color(236, 240, 241);  ///< The color of the thumb when the mouse is hovering over it.
    Color thumbBorderColor_ = Color(52, 152, 219);  ///< The color of the border around the thumb.
    
    float trackHeight_ = 4.0f;          ///< The thickness of the slider track, in pixels.
    float thumbRadius_ = 10.0f;         ///< The radius of the thumb, in pixels.
    float thumbBorderWidth_ = 2.0f;     ///< The thickness of the border around the thumb, in pixels.
    
    // Callback
    ValueChangedCallback onValueChanged_; ///< The callback function to call when the value changes.

public:
    /**
     * @brief Constructs a new Slider object.
     * @param[in] orientation The initial orientation of the slider (Horizontal or Vertical).
     */
    explicit Slider(Orientation orientation = Orientation::Horizontal);
    ~Slider() override;

    // Value
    /**
     * @brief Sets the current value of the slider.
     * @details The value will be clamped to the slider's current range [min, max].
     * @param[in] value The new value to set.
     */
    void setValue(double value);

    /**
     * @brief Gets the current value of the slider.
     * @return The current value.
     */
    double getValue() const noexcept { return value_; }
    
    /**
     * @brief Sets the value range of the slider.
     * @param[in] min The minimum selectable value.
     * @param[in] max The maximum selectable value.
     */
    void setRange(double min, double max);

    /**
     * @brief Gets the minimum value of the slider's range.
     * @return The minimum value.
     */
    double getMinValue() const noexcept { return minValue_; }

    /**
     * @brief Gets the maximum value of the slider's range.
     * @return The maximum value.
     */
    double getMaxValue() const noexcept { return maxValue_; }
    
    /**
     * @brief Sets the step increment.
     * @param[in] step The step value. Set to 0 for a continuous slider.
     */
    void setStep(double step) noexcept { step_ = step; }

    /**
     * @brief Gets the current step increment.
     * @return The step value.
     */
    double getStep() const noexcept { return step_; }
    
    /**
     * @brief Gets the current value normalized to the range [0.0, 1.0].
     * @return The normalized value. Returns 0 if max <= min.
     */
    double getNormalizedValue() const noexcept {
        if (maxValue_ <= minValue_) return 0.0;
        return (value_ - minValue_) / (maxValue_ - minValue_);
    }

    // Orientation
    /**
     * @brief Sets the orientation of the slider.
     * @param[in] orientation The new orientation (Horizontal or Vertical).
     */
    void setOrientation(Orientation orientation) noexcept { orientation_ = orientation; }

    /**
     * @brief Gets the current orientation of the slider.
     * @return The current orientation.
     */
    Orientation getOrientation() const noexcept { return orientation_; }

    // Enabled state
    /**
     * @brief Enables or disables the slider.
     * @param[in] enabled Set to true to enable interaction, false to disable.
     */
    void setEnabled(bool enabled) noexcept { enabled_ = enabled; }

    /**
     * @brief Checks if the slider is enabled.
     * @return True if the slider is enabled, false otherwise.
     */
    bool isEnabled() const noexcept { return enabled_; }

    // Styling
    void setTrackColor(const Color& color) noexcept { trackColor_ = color; }
    void setFillColor(const Color& color) noexcept { fillColor_ = color; }
    void setThumbColor(const Color& color) noexcept { thumbColor_ = color; }
    void setThumbHoverColor(const Color& color) noexcept { thumbHoverColor_ = color; }
    
    void setTrackHeight(float height) noexcept { trackHeight_ = height; }
    void setThumbRadius(float radius) noexcept { thumbRadius_ = radius; }

    // Callback
    /**
     * @brief Sets the callback function to be invoked when the slider's value changes.
     * @param[in] callback The function to call.
     */
    void setOnValueChanged(ValueChangedCallback callback) { 
        onValueChanged_ = std::move(callback); 
    }

    // Event handling
    bool onEvent(const event::Event& event) override;

    // Rendering
    void render(Renderer& renderer) override;

private:
    /**
     * @brief Handles mouse button press and release events.
     * @param[in] evt The mouse button event.
     * @return True if the event was handled.
     */
    bool handleMouseButton(const event::MouseButtonEvent& evt);

    /**
     * @brief Handles mouse movement events for dragging the thumb.
     * @param[in] evt The mouse move event.
     * @return True if the event was handled.
     */
    bool handleMouseMove(const event::MouseMoveEvent& evt);
    
    /**
     * @brief Updates the slider's value based on a mouse cursor position.
     * @param[in] point The point from a mouse event.
     */
    void updateValueFromPoint(const Point<int32_t>& point);

    /**
     * @brief Calculates the center position of the thumb based on the current value.
     * @return The center point of the thumb in widget coordinates.
     */
    Point<int32_t> getThumbPosition() const;

    /**
     * @brief Checks if a point is inside the slider's thumb.
     * @param[in] point The point to check.
     * @return True if the point is inside the thumb, false otherwise.
     */
    bool isPointInThumb(const Point<int32_t>& point) const;
    
    /**
     * @brief Snaps a given value to the nearest valid step.
     * @param[in] value The value to snap.
     * @return The snapped value.
     */
    double snapToStep(double value) const;

    /**
     * @brief Invokes the onValueChanged callback if it is set.
     */
    void notifyValueChanged();
};

} // namespace frqs::widget