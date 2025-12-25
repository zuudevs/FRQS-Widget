#pragma once

// ============================================================================
// FRQS-Widget: High-Performance Modern C++ GUI Framework
// Version: 0.1.0
// Language: C++23
// Platform: Windows 10/11 (Direct2D)
// License: MIT
// ============================================================================

// Single-include header for library users

// Meta layer (concepts & traits)
#include "meta/concepts.hpp"

// Geometry units (already implemented)
#include "unit/point.hpp"
#include "unit/size.hpp"
#include "unit/rect.hpp"
#include "unit/color.hpp"

// Platform abstraction (Windows-safe)
#include "platform/win32_safe.hpp"
#include "platform/message_queue.hpp"

// Event system
#include "event/event.hpp"
#include "event/event_bus.hpp"

// Core infrastructure (order matters!)
#include "core/window_id.hpp"        // Must come before window.hpp
#include "core/window.hpp"
#include "core/window_registry.hpp"
#include "core/application.hpp"

// Widget system
#include "widget/iwidget.hpp"
#include "widget/widget.hpp"
#include "widget/layout.hpp"
#include "widget/container.hpp"
#include "widget/button.hpp"
#include "widget/image.hpp"
#include "widget/label.hpp"
#include "widget/list_view.hpp"
#include "widget/scroll_view.hpp"
#include "widget/text_input.hpp"
#include "widget/label.hpp"
#include "widget/slider.hpp"
#include "widget/internal.hpp"
#include "widget/list_adapter.hpp"
#include "widget/checkbox.hpp"
#include "widget/combobox.hpp"

// Rendering
#include "render/dirty_rect.hpp"

// ============================================================================
// NAMESPACE ALIASES (Optional convenience)
// ============================================================================

namespace frqs {
    // Shorter alias for widget namespace
    namespace ui = widget;
    
    // Export commonly used types to main namespace
    using widget::Point;
    using widget::Size;
    using widget::Rect;
    using widget::Color;
    
    using core::Application;
    using core::Window;
    using core::WindowParams;
    using core::WindowId;
}

// ============================================================================
// VERSION MACROS
// ============================================================================

#define FRQS_WIDGET_VERSION_MAJOR 0
#define FRQS_WIDGET_VERSION_MINOR 1
#define FRQS_WIDGET_VERSION_PATCH 0

#define FRQS_WIDGET_VERSION \
    ((FRQS_WIDGET_VERSION_MAJOR * 10000) + \
     (FRQS_WIDGET_VERSION_MINOR * 100) + \
     FRQS_WIDGET_VERSION_PATCH)

// ============================================================================
// FEATURE DETECTION
// ============================================================================

#if __cplusplus < 202302L
    #error "FRQS-Widget requires C++23 or later"
#endif

#ifdef _MSC_VER
    #if _MSC_VER < 1938
        #error "MSVC 19.38+ (Visual Studio 2022 17.8+) required"
    #endif
#endif

// ============================================================================
// USAGE EXAMPLE
// ============================================================================

/*

#include <frqs-widget.hpp>

int main() {
    using namespace frqs;
    
    // Initialize application
    auto& app = Application::instance();
    app.initialize();
    
    // Create window
    WindowParams params;
    params.title = L"Hello FRQS";
    params.size = Size(800u, 600u);
    
    auto window = app.createWindow(params);
    window->show();
    
    // Run event loop
    app.run();
    
    return 0;
}

*/