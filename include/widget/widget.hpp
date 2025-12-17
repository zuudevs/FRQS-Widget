#pragma once

#include "iwidget.hpp"

namespace frqs::widget {

// Note: LayoutProps is defined in iwidget.hpp

// ============================================================================
// WIDGET HELPER FUNCTIONS
// ============================================================================

// Check if point is inside widget rect
[[nodiscard]] inline bool isPointInside(
    const IWidget* widget,
    const Point<int32_t>& point
) noexcept {
    if (!widget) return false;
    
    auto rect = widget->getRect();
    return point.x >= rect.x &&
           point.x < rect.getRight() &&
           point.y >= rect.y &&
           point.y < rect.getBottom();
}

// Find widget at position (recursive search)
[[nodiscard]] inline IWidget* findWidgetAt(
    IWidget* root,
    const Point<int32_t>& point
) noexcept {
    if (!root || !root->isVisible()) return nullptr;
    
    // Check children first (front to back)
    for (const auto& child : root->getChildren()) {
        if (auto found = findWidgetAt(child.get(), point)) {
            return found;
        }
    }
    
    // Check self
    if (isPointInside(root, point)) {
        return root;
    }
    
    return nullptr;
}

// Visit all widgets in tree (depth-first)
template <typename Visitor>
void visitWidgets(IWidget* root, Visitor&& visitor) {
    if (!root) return;
    
    visitor(root);
    
    for (const auto& child : root->getChildren()) {
        visitWidgets(child.get(), std::forward<Visitor>(visitor));
    }
}

// Visit visible widgets only
template <typename Visitor>
void visitVisibleWidgets(IWidget* root, Visitor&& visitor) {
    if (!root || !root->isVisible()) return;
    
    visitor(root);
    
    for (const auto& child : root->getChildren()) {
        if (child->isVisible()) {
            visitVisibleWidgets(child.get(), std::forward<Visitor>(visitor));
        }
    }
}

// ============================================================================
// LAYOUT PROPERTY ACCESSORS (for Widget class)
// ============================================================================

// These are implemented in widget.cpp
// Usage: Cast IWidget* to Widget* to access layout properties

} // namespace frqs::widget