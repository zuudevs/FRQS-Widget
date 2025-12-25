/**
 * @file dirty_rect.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines a manager for tracking and optimizing redraw regions (dirty rectangles).
 * @version 0.1
 * @date 2025-12-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <vector>
#include "unit/rect.hpp"

namespace frqs::render {

// ============================================================================
// DIRTY RECT MANAGER (Partial Redraw Optimization)
// ============================================================================

/**
 * @class DirtyRectManager
 * @brief Manages dirty rectangles for partial screen redraws.
 * 
 * This class tracks regions of a window that need to be redrawn, allowing the
 * renderer to update only the changed parts of the screen instead of the entire
 * window. This significantly improves performance, especially for small, frequent updates.
 * It automatically merges overlapping or adjacent rectangles and can trigger a full
 * redraw if the dirty area becomes too large.
 */
class DirtyRectManager {
private:
    std::vector<widget::Rect<int32_t, uint32_t>> dirtyRects_; ///< A list of rectangles that need redrawing.
    widget::Rect<int32_t, uint32_t> bounds_;  ///< The total bounds of the area being managed (e.g., the window).
    bool fullRedraw_ = false; ///< Flag indicating if a full redraw is required.

    /**
     * @brief Threshold for switching to a full redraw.
     * If the total area of dirty rectangles exceeds this percentage of the total
     * bounds, a full redraw is performed instead of many small ones.
     */
    static constexpr float FULL_REDRAW_THRESHOLD = 0.75f;  // 75% of window

public:
    /**
     * @brief Constructs a DirtyRectManager.
     * @param bounds The initial bounds of the area to manage (e.g., window size).
     */
    explicit DirtyRectManager(const widget::Rect<int32_t, uint32_t>& bounds)
        : bounds_(bounds) {}

    /**
     * @brief Marks a specific region as dirty and needing a redraw.
     * The provided rectangle is clipped to the manager's bounds. The method
     * attempts to merge the new rectangle with existing dirty rectangles to
     * minimize the number of separate redraw operations.
     * 
     * @param rect The rectangle to mark as dirty.
     */
    void addDirtyRect(const widget::Rect<int32_t, uint32_t>& rect) {
        if (fullRedraw_) return;  // Already doing full redraw

        // Clip to bounds
        auto clipped = rect.intersect(bounds_);
        if (clipped.w == 0 || clipped.h == 0) return;  // Outside bounds

        // Try to merge with existing rects to reduce overdraw
        bool merged = false;
        for (auto& existing : dirtyRects_) {
            if (shouldMerge(existing, clipped)) {
                existing = merge(existing, clipped);
                merged = true;
                break;
            }
        }

        if (!merged) {
            dirtyRects_.push_back(clipped);
        }

        // Check if dirty area exceeds threshold
        if (getDirtyAreaRatio() > FULL_REDRAW_THRESHOLD) {
            markFullRedraw();
        }
    }

    /**
     * @brief Forces the entire managed area to be marked for a full redraw.
     * This clears any existing dirty rectangles.
     */
    void markFullRedraw() noexcept {
        dirtyRects_.clear();
        fullRedraw_ = true;
    }

    /**
     * @brief Retrieves the list of rectangles that need to be redrawn.
     * @return A vector of dirty rectangles. If a full redraw is needed, this
     *         vector contains a single rectangle covering the entire bounds.
     */
    [[nodiscard]] std::vector<widget::Rect<int32_t, uint32_t>> getDirtyRects() const {
        if (fullRedraw_) {
            return {bounds_};
        }
        return dirtyRects_;
    }

    /**
     * @brief Checks if any part of the area is dirty.
     * @return `true` if there are any dirty rectangles or if a full redraw is pending, `false` otherwise.
     */
    [[nodiscard]] bool isDirty() const noexcept {
        return fullRedraw_ || !dirtyRects_.empty();
    }

    /**
     * @brief Checks if a full redraw is currently needed.
     * @return `true` if a full redraw is pending, `false` otherwise.
     */
    [[nodiscard]] bool needsFullRedraw() const noexcept {
        return fullRedraw_;
    }

    /**
     * @brief Clears all dirty rectangles and resets the full redraw flag.
     * This should be called after a render pass is complete.
     */
    void clear() noexcept {
        dirtyRects_.clear();
        fullRedraw_ = false;
    }

    /**
     * @brief Updates the bounds of the managed area.
     * This is typically called when the window is resized. It automatically
     * triggers a full redraw.
     * @param bounds The new bounds.
     */
    void setBounds(const widget::Rect<int32_t, uint32_t>& bounds) noexcept {
        bounds_ = bounds;
        markFullRedraw();  // Full redraw on resize
    }

private:
    /**
     * @brief Calculates the ratio of the total dirty area to the total bounds area.
     * @return The ratio as a float (0.0 to 1.0).
     */
    [[nodiscard]] float getDirtyAreaRatio() const noexcept {
        if (dirtyRects_.empty()) return 0.0f;

        uint64_t dirtyArea = 0;
        for (const auto& rect : dirtyRects_) {
            dirtyArea += static_cast<uint64_t>(rect.w) * rect.h;
        }

        uint64_t totalArea = static_cast<uint64_t>(bounds_.w) * bounds_.h;
        return totalArea > 0 ? static_cast<float>(dirtyArea) / totalArea : 0.0f;
    }

    /**
     * @brief Determines if two rectangles should be merged.
     * Merging is recommended if the "wasted space" in the resulting bounding
     * box is reasonably small compared to the combined area of the two rectangles.
     * 
     * @param a The first rectangle.
     * @param b The second rectangle.
     * @return `true` if the rectangles should be merged, `false` otherwise.
     */
    [[nodiscard]] bool shouldMerge(
        const widget::Rect<int32_t, uint32_t>& a,
        const widget::Rect<int32_t, uint32_t>& b
    ) const noexcept {
        // Calculate the bounding box
        auto merged = merge(a, b);
        uint64_t mergedArea = static_cast<uint64_t>(merged.w) * merged.h;
        uint64_t areaA = static_cast<uint64_t>(a.w) * a.h;
        uint64_t areaB = static_cast<uint64_t>(b.w) * b.h;

        // Merge if the wasted area is less than 25% of total
        uint64_t wastedArea = mergedArea - areaA - areaB;
        return wastedArea < (areaA + areaB) / 4;
    }

    /**
     * @brief Merges two rectangles into a single bounding box that contains both.
     * @param a The first rectangle.
     * @param b The second rectangle.
     * @return A new rectangle that is the union of the two input rectangles.
     */
    [[nodiscard]] widget::Rect<int32_t, uint32_t> merge(
        const widget::Rect<int32_t, uint32_t>& a,
        const widget::Rect<int32_t, uint32_t>& b
    ) const noexcept {
        auto left = std::min(a.getLeft(), b.getLeft());
        auto top = std::min(a.getTop(), b.getTop());
        auto right = std::max(a.getRight(), b.getRight());
        auto bottom = std::max(a.getBottom(), b.getBottom());

        return widget::Rect<int32_t, uint32_t>(
            left, top,
            static_cast<uint32_t>(right - left),
            static_cast<uint32_t>(bottom - top)
        );
    }
};

} // namespace frqs::render