/**
 * @file internal.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines internal functions for the widget library.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

namespace frqs::widget {

class Widget;

/**
 * @brief Contains internal implementation details for the widget system.
 * @warning This namespace is for internal use only. Its contents are subject to change without notice and should not be used directly in user code.
 */
namespace internal {

/**
 * @brief Recursively sets the native window handle for a widget and all its children.
 * 
 * This function is intended for internal use by the `Window` class when a widget tree is attached to it.
 * It traverses the widget hierarchy starting from the given widget and assigns the window handle (`HWND` on Windows)
 * to each widget. This association is crucial for widgets to be able to interact with the parent window,
 * for example, to invalidate their region and trigger a redraw using `InvalidateRect`.
 * 
 * @param widget A pointer to the root widget of the tree (or subtree) to which the window handle will be set.
 * @param hwnd A type-erased pointer to the native window handle (e.g., `HWND`).
 */
void setWidgetWindowHandle(Widget* widget, void* hwnd);

} // namespace internal
} // namespace frqs::widget