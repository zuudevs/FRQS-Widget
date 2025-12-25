/**
 * @file window_impl.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2025-12-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "core/window.hpp"
#include "platform/win32_safe.hpp"
#include "render/dirty_rect.hpp"
#include "render/renderer_d2d.hpp"
#include <memory>

namespace frqs::core {

// ============================================================================
// WINDOW PIMPL IMPLEMENTATION (Private)
// ============================================================================

/**
 * @struct Window::Impl
 * @brief The private implementation of the Window class.
 *
 * This struct holds all the data and logic that is specific to the
 * platform implementation (in this case, Win32 and Direct2D). By hiding
 * these details from the public `window.hpp` header, we achieve a clean
 * separation of interface and implementation, reduce compile times, and
 * prevent platform-specific headers from leaking into client code.
 */
struct Window::Impl {
    // --- Platform and Rendering ---
    /** @brief The native handle to the window (e.g., HWND on Windows). */
    platform::NativeHandle hwnd = nullptr;
    /** @brief The Direct2D renderer responsible for drawing in this window. */
    std::unique_ptr<render::RendererD2D> renderer;
    /** @brief Manages the regions of the window that need to be redrawn. */
    std::unique_ptr<render::DirtyRectManager> dirtyRects;

    // --- Window Properties ---
    /** @brief The text displayed in the window's title bar. */
    std::wstring title = L"FRQS Window";
    /** @brief The current dimensions of the window's client area. */
    widget::Size<uint32_t> size {800, 600};
    /** @brief The current screen coordinates of the window's top-left corner. */
    widget::Point<int32_t> position {100, 100};
    
    // --- Widget Hierarchy ---
    /** @brief The root widget of the UI hierarchy contained within this window. */
    std::shared_ptr<widget::IWidget> rootWidget;
    
    // --- State Flags ---
    /** @brief `true` if the window is currently visible. */
    bool visible = false;
    /** @brief `true` if the user can resize the window. */
    bool resizable = true;
    /** @brief `true` if the window has a standard title bar and border. */
    bool decorated = true;
    /** @brief `true` if the window has keyboard focus. */
    bool focused = false;
    /** @brief `true` if the window is minimized. */
    bool minimized = false;
    /** @brief `true` if the window is maximized. */
    bool maximized = false;
    /** @brief `true` if the window has been closed and is pending destruction. */
    bool closed = false;
    /** @brief `true` while the user is actively resizing or moving the window. */
    bool inSizeMove = false;

    Impl() = default;
    ~Impl() noexcept = default;

    /**
     * @brief Initializes the dirty rectangle manager with the window's initial size.
     */
    void initializeDirtyRects() {
        widget::Rect<int32_t, uint32_t> bounds(0, 0, size.w, size.h);
        dirtyRects = std::make_unique<render::DirtyRectManager>(bounds);
    }

    /**
     * @brief Initializes the D2D renderer. Must be called after the native window handle (hwnd) is created.
     */
    void initializeRenderer() {
        if (hwnd && !renderer) {
            try {
                renderer = std::make_unique<render::RendererD2D>(hwnd);
            } catch (const std::exception& e) {
                // TODO: Log the error properly.
                // For now, we just swallow the exception to prevent a crash if D2D fails.
                (void)e;
            }
        }
    }

    /**
     * @brief Updates the bounds of the renderer and dirty rect manager when the window is resized.
     */
    void updateDirtyRectBounds() {
        if (dirtyRects) {
            widget::Rect<int32_t, uint32_t> bounds(0, 0, size.w, size.h);
            dirtyRects->setBounds(bounds);
        }
        
        // Resize the renderer's device-dependent resources to match the new window size.
        if (renderer) {
            renderer->resize(size.w, size.h);
        }
    }
    
    /**
     * @brief Handles a resize event from the OS.
     * @param newWidth The new width of the client area.
     * @param newHeight The new height of the client area.
     */
    void handleSizeMessage(uint32_t newWidth, uint32_t newHeight) {
        if (size.w == newWidth && size.h == newHeight) {
            return; // No actual size change
        }

        size = widget::Size<uint32_t>(newWidth, newHeight);
        updateDirtyRectBounds();
        
        // The root widget always fills the entire client area.
        if (rootWidget) {
            widget::Rect<int32_t, uint32_t> clientRect(0, 0, newWidth, newHeight);
            rootWidget->setRect(clientRect);
        }
        
        // Avoid rendering during interactive resize/move to prevent visual artifacts.
        if (!inSizeMove) {
            render();
        }
    }
    
    /**
     * @brief Renders the window's content.
     *
     * The rendering pipeline is as follows:
     * 1. Checks if rendering is possible (renderer and root widget exist, window is visible).
     * 2. Begins a Direct2D drawing session.
     * 3. Clears the background with a default color.
     * 4. Traverses the widget tree, telling each widget to render itself.
     * 5. Ends the Direct2D drawing session, presenting the final image.
     * 6. Clears the list of dirty rectangles, as they have now been repainted.
     */
    void render() {
        if (!renderer || !rootWidget || !visible || minimized) return;
        
        renderer->beginRender();
        
        // Clear the entire render target with a background color.
        renderer->clear(widget::Color(240, 240, 245));
        
        // Recursively render the widget tree, starting from the root.
        rootWidget->render(*renderer);
        
        renderer->endRender();
        
        // All invalid regions have been redrawn, so clear the dirty rects.
        if (dirtyRects) {
            dirtyRects->clear();
        }
    }
};

} // namespace frqs::core