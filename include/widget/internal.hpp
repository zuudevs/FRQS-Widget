#pragma once

namespace frqs::widget {

class Widget;

// Internal namespace for implementation details
// DO NOT USE IN USER CODE
namespace internal {

// Set window handle to widget tree (used by Window class)
// This enables widgets to trigger window redraws via InvalidateRect
void setWidgetWindowHandle(Widget* widget, void* hwnd);

} // namespace internal
} // namespace frqs::widget