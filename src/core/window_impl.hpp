#pragma once

#include "../../include/core/window.hpp"
#include "../../include/platform/win32_safe.hpp"
#include "../../include/render/dirty_rect.hpp"
#include "../render/renderer_d2d.hpp"
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
    
    // Rendering (FIX: Add renderer instance)
    std::unique_ptr<render::RendererD2D> renderer;
    std::unique_ptr<render::DirtyRectManager> dirtyRects;
    
    // State
    bool focused = false;
    bool minimized = false;
    bool maximized = false;
    bool closed = false;
    bool inSizeMove = false;  // Track resize/move state

    Impl() = default;
    ~Impl() noexcept = default;

    // Initialize dirty rect manager
    void initializeDirtyRects() {
        widget::Rect<int32_t, uint32_t> bounds(0, 0, size.w, size.h);
        dirtyRects = std::make_unique<render::DirtyRectManager>(bounds);
    }

    // Initialize renderer (called after HWND is created)
    void initializeRenderer() {
        if (hwnd && !renderer) {
            try {
                renderer = std::make_unique<render::RendererD2D>(hwnd);
            } catch (const std::exception& e) {
                // Log error but don't crash
                (void)e;
            }
        }
    }

    // Update dirty rect bounds on resize
    void updateDirtyRectBounds() {
        if (dirtyRects) {
            widget::Rect<int32_t, uint32_t> bounds(0, 0, size.w, size.h);
            dirtyRects->setBounds(bounds);
        }
        
        // FIX: Resize renderer buffer to match window size
        if (renderer) {
            renderer->resize(size.w, size.h);
        }
    }
    
    // Handle WM_SIZE message - update size and dependent systems
    void handleSizeMessage(uint32_t newWidth, uint32_t newHeight) {
        size = widget::Size<uint32_t>(newWidth, newHeight);
        updateDirtyRectBounds();
        
        if (rootWidget) {
            widget::Rect<int32_t, uint32_t> clientRect(0, 0, newWidth, newHeight);
            rootWidget->setRect(clientRect);
        }
        
        if (!inSizeMove) {
            render();
        }
    }
    
    // Render the window content
    void render() {
        if (!renderer || !rootWidget || !visible) return;
        
        // Begin rendering
        renderer->beginRender();
        
        // Clear background
        renderer->clear(widget::Color(240, 240, 245));
        
        // Render widget tree
        rootWidget->render(*renderer);
        
        // End rendering
        renderer->endRender();
        
        // Clear dirty rects after successful render
        if (dirtyRects) {
            dirtyRects->clear();
        }
    }
};

} // namespace frqs::core