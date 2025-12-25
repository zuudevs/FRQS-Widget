/**
 * @file widget.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Provides helper functions and utilities for working with the widget hierarchy.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "iwidget.hpp"

namespace frqs::widget {

// Note: LayoutProps is defined in iwidget.hpp

// ============================================================================
// WIDGET HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Checks if a given point is located inside the bounds of a widget.
 * @details The check is inclusive of the top and left edges, and exclusive of the
 * bottom and right edges.
 * @param[in] widget A pointer to the widget to check against.
 * @param[in] point The point to check, in the same coordinate system as the widget.
 * @return true if the point is inside the widget's rectangle, false otherwise.
 */
[[nodiscard]] inline bool isPointInside(
    const IWidget* widget,
    const Point<int32_t>& point
) noexcept {
    if (!widget) return false;
    
    auto rect = widget->getRect();
    return point.x >= static_cast<int32_t>(rect.x) &&
           point.x < static_cast<int32_t>(rect.getRight()) &&
           point.y >= static_cast<int32_t>(rect.y) &&
           point.y < static_cast<int32_t>(rect.getBottom());
}

/**
 * @brief Finds the topmost widget at a specific point by performing a hit test.
 * @deprecated This function is deprecated. Use IWidget::hitTest() directly on the root widget instead.
 * @param[in] root The root widget of the tree to search from.
 * @param[in] point The point to test.
 * @return A pointer to the topmost widget found at the point, or `nullptr`.
 */
[[nodiscard]] inline IWidget* findWidgetAt(
    IWidget* root,
    const Point<int32_t>& point
) noexcept {
    if (!root) return nullptr;
    return root->hitTest(point);
}

/**
 * @brief Traverses the widget tree in a depth-first order and applies a visitor function to each widget.
 * @tparam Visitor The type of the visitor function or lambda, which must be callable with an `IWidget*`.
 * @param[in] root The root widget to start the traversal from.
 * @param[in] visitor The visitor function to apply to each widget.
 */
template <typename Visitor>
void visitWidgets(IWidget* root, Visitor&& visitor) {
    if (!root) return;
    
    visitor(root);
    
    for (const auto& child : root->getChildren()) {
        visitWidgets(child.get(), std::forward<Visitor>(visitor));
    }
}

/**
 * @brief Traverses the widget tree, applying a visitor function only to visible widgets and their visible children.
 * @details This is useful for tasks like rendering or event handling where invisible widgets should be skipped.
 * @tparam Visitor The type of the visitor function or lambda, which must be callable with an `IWidget*`.
 * @param[in] root The root widget to start the traversal from.
 * @param[in] visitor The visitor function to apply to each visible widget.
 */
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