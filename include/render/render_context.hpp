#pragma once

#include "unit/rect.hpp"
#include <stack>

namespace frqs::render {

// ============================================================================
// RENDER STATE (Saved/Restored)
// ============================================================================

struct RenderState {
    widget::Rect<int32_t, uint32_t> clipRect;
    float opacity = 1.0f;
    
    // Transform matrix (2D affine)
    struct Transform {
        float m11 = 1.0f, m12 = 0.0f;
        float m21 = 0.0f, m22 = 1.0f;
        float dx = 0.0f, dy = 0.0f;
    } transform;
};

// ============================================================================
// RENDER CONTEXT (Stack-based state management)
// ============================================================================

class RenderContext {
private:
    std::stack<RenderState> stateStack_;
    RenderState currentState_;

public:
    RenderContext(const widget::Rect<int32_t, uint32_t>& initialClip) {
        currentState_.clipRect = initialClip;
    }

    // ========================================================================
    // STATE MANAGEMENT
    // ========================================================================

    void save() {
        stateStack_.push(currentState_);
    }

    void restore() {
        if (!stateStack_.empty()) {
            currentState_ = stateStack_.top();
            stateStack_.pop();
        }
    }

    // ========================================================================
    // CLIP REGION
    // ========================================================================

    void pushClip(const widget::Rect<int32_t, uint32_t>& rect) {
        save();
        currentState_.clipRect = currentState_.clipRect.intersect(rect);
    }

    void popClip() {
        restore();
    }

    [[nodiscard]] const widget::Rect<int32_t, uint32_t>& getClipRect() const noexcept {
        return currentState_.clipRect;
    }

    // ========================================================================
    // OPACITY
    // ========================================================================

    void setOpacity(float opacity) noexcept {
        currentState_.opacity = std::clamp(opacity, 0.0f, 1.0f);
    }

    [[nodiscard]] float getOpacity() const noexcept {
        return currentState_.opacity;
    }

    // ========================================================================
    // TRANSFORM
    // ========================================================================

    void setTransform(float m11, float m12, float m21, float m22,
                     float dx, float dy) noexcept {
        currentState_.transform = {m11, m12, m21, m22, dx, dy};
    }

    [[nodiscard]] const RenderState::Transform& getTransform() const noexcept {
        return currentState_.transform;
    }

    void translate(float dx, float dy) noexcept {
        currentState_.transform.dx += dx;
        currentState_.transform.dy += dy;
    }

    void scale(float sx, float sy) noexcept {
        currentState_.transform.m11 *= sx;
        currentState_.transform.m22 *= sy;
    }

    void rotate(float angle) noexcept {
        float cos_a = std::cos(angle);
        float sin_a = std::sin(angle);
        
        auto& t = currentState_.transform;
        float new_m11 = t.m11 * cos_a - t.m12 * sin_a;
        float new_m12 = t.m11 * sin_a + t.m12 * cos_a;
        float new_m21 = t.m21 * cos_a - t.m22 * sin_a;
        float new_m22 = t.m21 * sin_a + t.m22 * cos_a;
        
        t.m11 = new_m11;
        t.m12 = new_m12;
        t.m21 = new_m21;
        t.m22 = new_m22;
    }

    // ========================================================================
    // UTILITY
    // ========================================================================

    [[nodiscard]] bool isVisible(const widget::Rect<int32_t, uint32_t>& rect) const noexcept {
        auto intersection = currentState_.clipRect.intersect(rect);
        return intersection.w > 0 && intersection.h > 0;
    }

    void reset(const widget::Rect<int32_t, uint32_t>& initialClip) {
        while (!stateStack_.empty()) {
            stateStack_.pop();
        }
        currentState_ = RenderState{};
        currentState_.clipRect = initialClip;
    }
};

// ============================================================================
// SCOPED STATE GUARD (RAII)
// ============================================================================

class ScopedRenderState {
private:
    RenderContext& context_;

public:
    explicit ScopedRenderState(RenderContext& context) : context_(context) {
        context_.save();
    }

    ~ScopedRenderState() {
        context_.restore();
    }

    // Non-copyable, non-movable
    ScopedRenderState(const ScopedRenderState&) = delete;
    ScopedRenderState& operator=(const ScopedRenderState&) = delete;
};

} // namespace frqs::render