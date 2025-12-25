/**
 * @file render_context.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines the graphics state and context for rendering operations.
 * @version 0.1
 * @date 2025-12-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "unit/rect.hpp"
#include <stack>
#include <algorithm>
#include <cmath>

namespace frqs::render {

// ============================================================================
// RENDER STATE (Saved/Restored)
// ============================================================================

/**
 * @struct RenderState
 * @brief Holds a snapshot of the rendering state, including clipping, opacity, and transforms.
 * 
 * This struct is used by `RenderContext` to save and restore the graphics state.
 */
struct RenderState {
    widget::Rect<int32_t, uint32_t> clipRect; ///< The current clipping rectangle.
    float opacity = 1.0f; ///< The current opacity, from 0.0 (transparent) to 1.0 (opaque).
    
    /**
     * @struct Transform
     * @brief Represents a 2D affine transformation matrix.
     */
    struct Transform {
        float m11 = 1.0f, m12 = 0.0f;
        float m21 = 0.0f, m22 = 1.0f;
        float dx = 0.0f, dy = 0.0f;
    } transform;
};

// ============================================================================
// RENDER CONTEXT (Stack-based state management)
// ============================================================================

/**
 * @class RenderContext
 * @brief Manages a stack of `RenderState` to handle nested transformations and clipping.
 * 
 * This class provides a high-level API for manipulating the graphics state,
 * such as setting clipping rectangles, adjusting opacity, and applying transformations.
 * It uses a stack to allow for saving and restoring states, which is essential
 * for correctly rendering nested UI components.
 */
class RenderContext {
private:
    std::stack<RenderState> stateStack_; ///< Stack for saving and restoring render states.
    RenderState currentState_; ///< The currently active render state.

public:
    /**
     * @brief Constructs a RenderContext with an initial clipping rectangle.
     * @param initialClip The rectangle that defines the initial visible area.
     */
    RenderContext(const widget::Rect<int32_t, uint32_t>& initialClip) {
        currentState_.clipRect = initialClip;
    }

    // ========================================================================
    // STATE MANAGEMENT
    // ========================================================================

    /**
     * @brief Saves the current rendering state onto the stack.
     * 
     * This is useful before applying a temporary state change (like a new clip rect or transform)
     * that should be reverted later.
     */
    void save() {
        stateStack_.push(currentState_);
    }

    /**
     * @brief Restores the rendering state from the top of the stack.
     * 
     * If the stack is not empty, the current state is replaced with the state
     * at the top of the stack, and that state is popped.
     */
    void restore() {
        if (!stateStack_.empty()) {
            currentState_ = stateStack_.top();
            stateStack_.pop();
        }
    }

    // ========================================================================
    // CLIP REGION
    // ========================================================================

    /**
     * @brief Pushes a new clipping rectangle.
     * 
     * The new clip rectangle is intersected with the current one. The previous
     * state is saved, so `popClip` can restore it.
     * @param rect The new rectangle to clip against.
     */
    void pushClip(const widget::Rect<int32_t, uint32_t>& rect) {
        save();
        currentState_.clipRect = currentState_.clipRect.intersect(rect);
    }

    /**
     * @brief Pops the current clipping rectangle, restoring the previous one.
     */
    void popClip() {
        restore();
    }

    /**
     * @brief Gets the current clipping rectangle.
     * @return A const reference to the current clip rectangle.
     */
    [[nodiscard]] const widget::Rect<int32_t, uint32_t>& getClipRect() const noexcept {
        return currentState_.clipRect;
    }

    // ========================================================================
    // OPACITY
    // ========================================================================

    /**
     * @brief Sets the current opacity.
     * The value is clamped between 0.0 and 1.0.
     * @param opacity The new opacity value.
     */
    void setOpacity(float opacity) noexcept {
        currentState_.opacity = std::clamp(opacity, 0.0f, 1.0f);
    }

    /**
     * @brief Gets the current opacity.
     * @return The current opacity value.
     */
    [[nodiscard]] float getOpacity() const noexcept {
        return currentState_.opacity;
    }

    // ========================================================================
    // TRANSFORM
    // ========================================================================

    /**
     * @brief Sets the current 2D affine transformation matrix.
     * @param m11, m12, m21, m22 The matrix components.
     * @param dx, dy The translation components.
     */
    void setTransform(float m11, float m12, float m21, float m22,
                     float dx, float dy) noexcept {
        currentState_.transform = {m11, m12, m21, m22, dx, dy};
    }

    /**
     * @brief Gets the current transformation matrix.
     * @return A const reference to the current transform.
     */
    [[nodiscard]] const RenderState::Transform& getTransform() const noexcept {
        return currentState_.transform;
    }

    /**
     * @brief Applies a translation to the current transform.
     * @param dx The horizontal translation offset.
     * @param dy The vertical translation offset.
     */
    void translate(float dx, float dy) noexcept {
        currentState_.transform.dx += dx;
        currentState_.transform.dy += dy;
    }

    /**
     * @brief Applies a scaling to the current transform.
     * @param sx The horizontal scaling factor.
     * @param sy The vertical scaling factor.
     */
    void scale(float sx, float sy) noexcept {
        currentState_.transform.m11 *= sx;
        currentState_.transform.m22 *= sy;
    }

    /**
     * @brief Applies a rotation to the current transform.
     * @param angle The rotation angle in radians.
     */
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

    /**
     * @brief Checks if a rectangle is at least partially visible within the current clip region.
     * @param rect The rectangle to check.
     * @return `true` if the rectangle intersects with the current clip rectangle, `false` otherwise.
     */
    [[nodiscard]] bool isVisible(const widget::Rect<int32_t, uint32_t>& rect) const noexcept {
        auto intersection = currentState_.clipRect.intersect(rect);
        return intersection.w > 0 && intersection.h > 0;
    }

    /**
     * @brief Resets the render context to its initial state.
     * 
     * This clears the state stack and resets the current state to a default,
     * using the provided rectangle as the initial clip region.
     * @param initialClip The new initial clipping rectangle.
     */
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

/**
 * @class ScopedRenderState
 * @brief An RAII guard for saving and restoring `RenderContext` state.
 * 
 * When a `ScopedRenderState` object is created, it saves the current state of
 * the provided `RenderContext`. When it is destroyed (at the end of its scope),
 * it automatically restores the saved state. This is a convenient and safe
 * way to manage temporary state changes.
 */
class ScopedRenderState {
private:
    RenderContext& context_;

public:
    /**
     * @brief Constructs a ScopedRenderState, saving the context's state.
     * @param context The `RenderContext` to manage.
     */
    explicit ScopedRenderState(RenderContext& context) : context_(context) {
        context_.save();
    }

    /**
     * @brief Destructor, restores the context's state.
     */
    ~ScopedRenderState() {
        context_.restore();
    }

    // Non-copyable, non-movable
    ScopedRenderState(const ScopedRenderState&) = delete;
    ScopedRenderState& operator=(const ScopedRenderState&) = delete;
};

} // namespace frqs::render