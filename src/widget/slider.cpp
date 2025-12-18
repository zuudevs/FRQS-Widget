#include "widget/slider.hpp"
#include <algorithm>
#include <cmath>

namespace frqs::widget {

// ============================================================================
// SLIDER PIMPL
// ============================================================================

struct Slider::Impl {
    Point<int32_t> dragStartPos;
    double dragStartValue;
};

// ============================================================================
// SLIDER IMPLEMENTATION
// ============================================================================

Slider::Slider(Orientation orientation)
    : Widget()
    , pImpl_(std::make_unique<Impl>())
    , orientation_(orientation)
{
    setBackgroundColor(colors::Transparent);
}

Slider::~Slider() = default;

void Slider::setValue(double value) {
    // Clamp and snap to step
    value = std::clamp(value, minValue_, maxValue_);
    value = snapToStep(value);
    
    if (std::abs(value_ - value) < 1e-10) return;
    
    value_ = value;
    invalidate();
}

void Slider::setRange(double min, double max) {
    if (min >= max) return;
    
    minValue_ = min;
    maxValue_ = max;
    
    // Clamp current value to new range
    setValue(value_);
}

double Slider::snapToStep(double value) const {
    if (step_ <= 0.0) return value;
    
    double steps = std::round((value - minValue_) / step_);
    return minValue_ + steps * step_;
}

void Slider::notifyValueChanged() {
    if (onValueChanged_) {
        onValueChanged_(value_);
    }
}

Point<int32_t> Slider::getThumbPosition() const {
    auto rect = getRect();
    double normalized = getNormalizedValue();
    
    if (orientation_ == Orientation::Horizontal) {
        // Horizontal slider
        int32_t trackWidth = static_cast<int32_t>(rect.w) - static_cast<int32_t>(thumbRadius_ * 2);
        int32_t thumbX = rect.x + static_cast<int32_t>(thumbRadius_) + 
                        static_cast<int32_t>(normalized * trackWidth);
        int32_t thumbY = rect.y + static_cast<int32_t>(rect.h / 2);
        
        return Point<int32_t>(thumbX, thumbY);
    } else {
        // Vertical slider
        int32_t trackHeight = static_cast<int32_t>(rect.h) - static_cast<int32_t>(thumbRadius_ * 2);
        int32_t thumbX = rect.x + static_cast<int32_t>(rect.w / 2);
        int32_t thumbY = rect.y + static_cast<int32_t>(thumbRadius_) + 
                        static_cast<int32_t>((1.0 - normalized) * trackHeight);
        
        return Point<int32_t>(thumbX, thumbY);
    }
}

bool Slider::isPointInThumb(const Point<int32_t>& point) const {
    auto thumbPos = getThumbPosition();
    
    int32_t dx = point.x - thumbPos.x;
    int32_t dy = point.y - thumbPos.y;
    int32_t distSq = dx * dx + dy * dy;
    int32_t radiusSq = static_cast<int32_t>(thumbRadius_ * thumbRadius_);
    
    return distSq <= radiusSq;
}

void Slider::updateValueFromPoint(const Point<int32_t>& point) {
    auto rect = getRect();
    double normalized;
    
    if (orientation_ == Orientation::Horizontal) {
        int32_t trackWidth = static_cast<int32_t>(rect.w) - static_cast<int32_t>(thumbRadius_ * 2);
        int32_t relX = point.x - rect.x - static_cast<int32_t>(thumbRadius_);
        
        normalized = static_cast<double>(relX) / trackWidth;
    } else {
        int32_t trackHeight = static_cast<int32_t>(rect.h) - static_cast<int32_t>(thumbRadius_ * 2);
        int32_t relY = point.y - rect.y - static_cast<int32_t>(thumbRadius_);
        
        normalized = 1.0 - (static_cast<double>(relY) / trackHeight);
    }
    
    normalized = std::clamp(normalized, 0.0, 1.0);
    double newValue = minValue_ + normalized * (maxValue_ - minValue_);
    
    setValue(newValue);
    notifyValueChanged();
}

bool Slider::handleMouseButton(const event::MouseButtonEvent& evt) {
    if (!enabled_) return false;
    
    if (evt.button == event::MouseButtonEvent::Button::Left) {
        if (evt.action == event::MouseButtonEvent::Action::Press) {
            auto rect = getRect();
            bool inside = evt.position.x >= rect.x && evt.position.x < static_cast<int32_t>(rect.getRight()) &&
                         evt.position.y >= rect.y && evt.position.y < static_cast<int32_t>(rect.getBottom());
            
            if (inside) {
                dragging_ = true;
                pImpl_->dragStartPos = evt.position;
                pImpl_->dragStartValue = value_;
                
                // Update value immediately on click
                updateValueFromPoint(evt.position);
                
                return true;
            }
        } else if (evt.action == event::MouseButtonEvent::Action::Release) {
            if (dragging_) {
                dragging_ = false;
                return true;
            }
        }
    }
    
    return false;
}

bool Slider::handleMouseMove(const event::MouseMoveEvent& evt) {
    if (!enabled_) return false;
    
    auto rect = getRect();
    bool inside = evt.position.x >= rect.x && evt.position.x < static_cast<int32_t>(rect.getRight()) &&
                 evt.position.y >= rect.y && evt.position.y < static_cast<int32_t>(rect.getBottom());
    
    if (dragging_) {
        updateValueFromPoint(evt.position);
        return true;
    } else {
        // Update hover state
        bool wasHovered = hovered_;
        hovered_ = inside && isPointInThumb(evt.position);
        
        if (wasHovered != hovered_) {
            invalidate();
        }
        
        return hovered_;
    }
}

bool Slider::onEvent(const event::Event& event) {
    if (auto* mouseBtn = std::get_if<event::MouseButtonEvent>(&event)) {
        return handleMouseButton(*mouseBtn);
    }
    
    if (auto* mouseMove = std::get_if<event::MouseMoveEvent>(&event)) {
        return handleMouseMove(*mouseMove);
    }
    
    return Widget::onEvent(event);
}

void Slider::render(Renderer& renderer) {
    if (!isVisible()) return;

    auto rect = getRect();
    auto thumbPos = getThumbPosition();
    
    // Render background
    Widget::render(renderer);
    
    if (orientation_ == Orientation::Horizontal) {
        // Horizontal track
        int32_t trackY = rect.y + static_cast<int32_t>(rect.h / 2 - trackHeight_ / 2);
        uint32_t trackH = static_cast<uint32_t>(trackHeight_);
        
        Rect<int32_t, uint32_t> trackRect(rect.x, trackY, rect.w, trackH);
        Rect<int32_t, uint32_t> fillRect(
            rect.x,
            trackY,
            static_cast<uint32_t>(thumbPos.x - rect.x),
            trackH
        );
        
        // Track background
        if (auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer)) {
            extRenderer->fillRoundedRect(trackRect, trackHeight_ / 2, trackHeight_ / 2, trackColor_);
            extRenderer->fillRoundedRect(fillRect, trackHeight_ / 2, trackHeight_ / 2, fillColor_);
        } else {
            renderer.fillRect(trackRect, trackColor_);
            renderer.fillRect(fillRect, fillColor_);
        }
    } else {
        // Vertical track
        int32_t trackX = rect.x + static_cast<int32_t>(rect.w / 2 - trackHeight_ / 2);
        uint32_t trackW = static_cast<uint32_t>(trackHeight_);
        
        Rect<int32_t, uint32_t> trackRect(trackX, rect.y, trackW, rect.h);
        Rect<int32_t, uint32_t> fillRect(
            trackX,
            thumbPos.y,
            trackW,
            static_cast<uint32_t>(rect.getBottom() - thumbPos.y)
        );
        
        // Track background
        if (auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer)) {
            extRenderer->fillRoundedRect(trackRect, trackHeight_ / 2, trackHeight_ / 2, trackColor_);
            extRenderer->fillRoundedRect(fillRect, trackHeight_ / 2, trackHeight_ / 2, fillColor_);
        } else {
            renderer.fillRect(trackRect, trackColor_);
            renderer.fillRect(fillRect, fillColor_);
        }
    }
    
    // Render thumb
    Rect<int32_t, uint32_t> thumbRect(
        thumbPos.x - static_cast<int32_t>(thumbRadius_),
        thumbPos.y - static_cast<int32_t>(thumbRadius_),
        static_cast<uint32_t>(thumbRadius_ * 2),
        static_cast<uint32_t>(thumbRadius_ * 2)
    );
    
    Color currentThumbColor = (hovered_ || dragging_) ? thumbHoverColor_ : thumbColor_;
    
    if (auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer)) {
        extRenderer->fillEllipse(thumbRect, currentThumbColor);
        extRenderer->drawEllipse(thumbRect, thumbBorderColor_, thumbBorderWidth_);
    } else {
        renderer.fillRect(thumbRect, currentThumbColor);
        renderer.drawRect(thumbRect, thumbBorderColor_, thumbBorderWidth_);
    }
}

} // namespace frqs::widget