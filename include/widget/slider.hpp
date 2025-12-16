#pragma once

#include "iwidget.hpp"
#include <functional>
#include <cmath>
#include "render/renderer.hpp"

namespace frqs::widget {

// ============================================================================
// SLIDER WIDGET (Horizontal/Vertical numeric slider)
// ============================================================================

class Slider : public Widget {
public:
    using ValueChangedCallback = std::function<void(double)>;

    enum class Orientation : uint8_t {
        Horizontal,
        Vertical
    };

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;

    // Value
    double value_ = 0.0;
    double minValue_ = 0.0;
    double maxValue_ = 100.0;
    double step_ = 1.0;  // 0 = continuous
    
    // State
    Orientation orientation_ = Orientation::Horizontal;
    bool dragging_ = false;
    bool hovered_ = false;
    bool enabled_ = true;
    
    // Styling
    Color trackColor_ = Color(189, 195, 199);
    Color fillColor_ = Color(52, 152, 219);
    Color thumbColor_ = colors::White;
    Color thumbHoverColor_ = Color(236, 240, 241);
    Color thumbBorderColor_ = Color(52, 152, 219);
    
    float trackHeight_ = 4.0f;
    float thumbRadius_ = 10.0f;
    float thumbBorderWidth_ = 2.0f;
    
    // Callback
    ValueChangedCallback onValueChanged_;

public:
    explicit Slider(Orientation orientation = Orientation::Horizontal);
    ~Slider() override;

    // Value
    void setValue(double value);
    double getValue() const noexcept { return value_; }
    
    void setRange(double min, double max);
    double getMinValue() const noexcept { return minValue_; }
    double getMaxValue() const noexcept { return maxValue_; }
    
    void setStep(double step) noexcept { step_ = step; }
    double getStep() const noexcept { return step_; }
    
    // Get normalized value [0.0, 1.0]
    double getNormalizedValue() const noexcept {
        if (maxValue_ <= minValue_) return 0.0;
        return (value_ - minValue_) / (maxValue_ - minValue_);
    }

    // Orientation
    void setOrientation(Orientation orientation) noexcept { orientation_ = orientation; }
    Orientation getOrientation() const noexcept { return orientation_; }

    // Enabled state
    void setEnabled(bool enabled) noexcept { enabled_ = enabled; }
    bool isEnabled() const noexcept { return enabled_; }

    // Styling
    void setTrackColor(const Color& color) noexcept { trackColor_ = color; }
    void setFillColor(const Color& color) noexcept { fillColor_ = color; }
    void setThumbColor(const Color& color) noexcept { thumbColor_ = color; }
    void setThumbHoverColor(const Color& color) noexcept { thumbHoverColor_ = color; }
    
    void setTrackHeight(float height) noexcept { trackHeight_ = height; }
    void setThumbRadius(float radius) noexcept { thumbRadius_ = radius; }

    // Callback
    void setOnValueChanged(ValueChangedCallback callback) { 
        onValueChanged_ = std::move(callback); 
    }

    // Event handling
    bool onEvent(const event::Event& event) override;

    // Rendering
    void render(Renderer& renderer) override;

private:
    bool handleMouseButton(const event::MouseButtonEvent& evt);
    bool handleMouseMove(const event::MouseMoveEvent& evt);
    
    void updateValueFromPoint(const Point<int32_t>& point);
    Point<int32_t> getThumbPosition() const;
    bool isPointInThumb(const Point<int32_t>& point) const;
    
    double snapToStep(double value) const;
    void notifyValueChanged();
};

} // namespace frqs::widget