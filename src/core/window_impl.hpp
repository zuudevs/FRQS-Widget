#pragma once

#include "../../include/core/window.hpp"
#include "../../include/platform/win32_safe.hpp"
#include "../../include/render/dirty_rect.hpp"
#include <memory>

namespace frqs::core {

// ============================================================================
// WINDOW PIMPL IMPLEMENTATION (Private)
// ============================================================================

struct Window::Impl {
    // Native window handle
    platform::NativeHandle hwnd = nullptr;
    
    // Window properties
    std::wstring title = L"FRQS Window";
    widget::Size<uint32_t> size {800, 600};
    widget::Point<int32_t> position {100, 100};
    bool visible = false;
    bool resizable = true;
    bool decorated = true;
    
    // Widget tree
    std::shared_ptr<widget::IWidget> rootWidget;
    
    // Rendering
    std::unique_ptr<render::DirtyRectManager> dirtyRects;
    
    // State
    bool focused = false;
    bool minimized = false;
    bool maximized = false;
    bool closed = false;

    Impl() = default;
    ~Impl() noexcept = default;

    // Initialize dirty rect manager
    void initializeDirtyRects() {
        widget::Rect<int32_t, uint32_t> bounds(0, 0, size.w, size.h);
        dirtyRects = std::make_unique<render::DirtyRectManager>(bounds);
    }

    // Update dirty rect bounds on resize
    void updateDirtyRectBounds() {
        if (dirtyRects) {
            widget::Rect<int32_t, uint32_t> bounds(0, 0, size.w, size.h);
            dirtyRects->setBounds(bounds);
        }
    }
};

} // namespace frqs::core