/**
 * @file dirty_rect.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2025-12-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "render/dirty_rect.hpp"

namespace frqs::render {

// ============================================================================
// DIRTY RECT MANAGER IMPLEMENTATION
// ============================================================================

// Note: DirtyRectManager is mostly implemented inline in the header
// This file exists for potential future non-inline implementations

// Example: Advanced optimization algorithms could go here
/*
void DirtyRectManager::optimizeDirtyRects() {
    // Implement more sophisticated merging algorithms
    // - Quadtree-based region merging
    // - Cost-benefit analysis for merge vs separate draws
    // - Heuristic-based full redraw detection
}
*/

// Helper: Calculate total dirty area
uint64_t calculateTotalDirtyArea(
    const std::vector<widget::Rect<int32_t, uint32_t>>& rects
) {
    uint64_t totalArea = 0;
    for (const auto& rect : rects) {
        totalArea += static_cast<uint64_t>(rect.w) * rect.h;
    }
    return totalArea;
}

// Helper: Check if rects overlap
bool rectsOverlap(
    const widget::Rect<int32_t, uint32_t>& a,
    const widget::Rect<int32_t, uint32_t>& b
) {
    return !(a.getRight() <= static_cast<uint32_t>(b.getLeft()) ||
             b.getRight() <= static_cast<uint32_t>(a.getLeft()) ||
             a.getBottom() <= static_cast<uint32_t>(b.getTop()) ||
             b.getBottom() <= static_cast<uint32_t>(a.getTop()));
}

// Helper: Calculate overlap area
uint64_t calculateOverlapArea(
    const widget::Rect<int32_t, uint32_t>& a,
    const widget::Rect<int32_t, uint32_t>& b
) {
    if (!rectsOverlap(a, b)) return 0;

    auto intersection = a.intersect(b);
    return static_cast<uint64_t>(intersection.w) * intersection.h;
}

// Debug: Print dirty rects info
void printDirtyRectsInfo(
    const std::vector<widget::Rect<int32_t, uint32_t>>& rects,
    const widget::Rect<int32_t, uint32_t>& bounds
) {
    if (rects.empty()) {
        // std::println("No dirty rects");
        return;
    }

    uint64_t totalArea = calculateTotalDirtyArea(rects);
    uint64_t boundsArea = static_cast<uint64_t>(bounds.w) * bounds.h;
    float percentage = boundsArea > 0 
        ? (static_cast<float>(totalArea) / boundsArea * 100.0f) 
        : 0.0f;

    // std::println("Dirty rects: {} regions, {:.2f}% of window", 
    //             rects.size(), percentage);
}

} // namespace frqs::render