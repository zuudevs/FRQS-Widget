#pragma once

#include <vector>
#include "../unit/rect.hpp"

namespace frqs::render {

// ============================================================================
// DIRTY RECT MANAGER (Partial Redraw Optimization)
// ============================================================================

class DirtyRectManager {
private:
    std::vector<widget::Rect<int32_t, uint32_t>> dirtyRects_;
    widget::Rect<int32_t, uint32_t> bounds_;  // Window bounds
    bool fullRedraw_ = false;

    // Merge threshold - if dirty area exceeds this, do full redraw
    static constexpr float FULL_REDRAW_THRESHOLD = 0.75f;  // 75% of window

public:
    explicit DirtyRectManager(const widget::Rect<int32_t, uint32_t>& bounds)
        : bounds_(bounds) {}

    // Mark a region as dirty
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

    // Mark entire window for redraw
    void markFullRedraw() noexcept {
        dirtyRects_.clear();
        fullRedraw_ = true;
    }

    // Get all dirty rects (or full window if needed)
    [[nodiscard]] std::vector<widget::Rect<int32_t, uint32_t>> getDirtyRects() const {
        if (fullRedraw_) {
            return {bounds_};
        }
        return dirtyRects_;
    }

    // Check if any region is dirty
    [[nodiscard]] bool isDirty() const noexcept {
        return fullRedraw_ || !dirtyRects_.empty();
    }

    // Check if full redraw is needed
    [[nodiscard]] bool needsFullRedraw() const noexcept {
        return fullRedraw_;
    }

    // Clear all dirty rects (call after rendering)
    void clear() noexcept {
        dirtyRects_.clear();
        fullRedraw_ = false;
    }

    // Update bounds (e.g., on window resize)
    void setBounds(const widget::Rect<int32_t, uint32_t>& bounds) noexcept {
        bounds_ = bounds;
        markFullRedraw();  // Full redraw on resize
    }

private:
    // Calculate dirty area as ratio of window area
    [[nodiscard]] float getDirtyAreaRatio() const noexcept {
        if (dirtyRects_.empty()) return 0.0f;

        uint64_t dirtyArea = 0;
        for (const auto& rect : dirtyRects_) {
            dirtyArea += static_cast<uint64_t>(rect.w) * rect.h;
        }

        uint64_t totalArea = static_cast<uint64_t>(bounds_.w) * bounds_.h;
        return totalArea > 0 ? static_cast<float>(dirtyArea) / totalArea : 0.0f;
    }

    // Check if two rects should be merged (overlap or adjacent)
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

    // Merge two rects into bounding box
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