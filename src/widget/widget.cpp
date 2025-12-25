/**
 * @file widget.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Implements the base Widget class for the UI framework.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "widget/iwidget.hpp"
#include "core/window.hpp"
#include "platform/win32_safe.hpp"
#include <algorithm>

namespace frqs::widget {

// ============================================================================
// WIDGET PIMPL IMPLEMENTATION
// ============================================================================

/**
 * @brief Private implementation details for the Widget class (PIMPL idiom).
 * 
 * This struct holds all the private data members for the Widget class,
 * hiding implementation details from the public header.
 * @struct Widget::Impl
 * @internal
 */
struct Widget::Impl {
    Rect<int32_t, uint32_t> rect;
    Color backgroundColor = colors::White;
    bool visible = true;
    IWidget* parent = nullptr;
    std::vector<std::shared_ptr<IWidget>> children;
    HWND windowHandle = nullptr;
    
    // Layout properties
    LayoutProps layoutProps;
    
    Impl() = default;
    
    /**
     * @brief Recursively finds the top-level window handle.
     * @return The HWND of the window containing this widget, or nullptr if not in a window.
     */
    HWND getWindowHandle() {
        if (windowHandle) return windowHandle;
        if (parent) {
            if (auto* parentWidget = dynamic_cast<Widget*>(parent)) {
                return parentWidget->pImpl_->getWindowHandle();
            }
        }
        return nullptr;
    }
};

// ============================================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================================

/**
 * @brief Constructs a new Widget.
 */
Widget::Widget() : pImpl_(std::make_unique<Impl>()) {}

/**
 * @brief Destroys the Widget.
 */
Widget::~Widget() noexcept = default;

/**
 * @brief Move constructor for Widget.
 */
Widget::Widget(Widget&& other) noexcept = default;

/**
 * @brief Move assignment operator for Widget.
 */
Widget& Widget::operator=(Widget&& other) noexcept = default;

// ============================================================================
// LAYOUT METHODS
// ============================================================================

/**
 * @brief Sets the position and size of the widget.
 * @param rect The new rectangle defining the widget's bounds.
 */
void Widget::setRect(const Rect<int32_t, uint32_t>& rect) {
    if (pImpl_->rect == rect) return;
    
    pImpl_->rect = rect;
    invalidate();
}

/**
 * @brief Gets the position and size of the widget.
 * @return The rectangle defining the widget's bounds.
 */
Rect<int32_t, uint32_t> Widget::getRect() const noexcept {
    return pImpl_->rect;
}

/**
 * @brief Sets the visibility of the widget.
 * @param visible `true` to make the widget visible, `false` to hide it.
 */
void Widget::setVisible(bool visible) noexcept {
    if (pImpl_->visible == visible) return;
    
    pImpl_->visible = visible;
    invalidate();
}

/**
 * @brief Gets the visibility of the widget.
 * @return `true` if the widget is visible, `false` otherwise.
 */
bool Widget::isVisible() const noexcept {
    return pImpl_->visible;
}

// ============================================================================
// HIT-TEST IMPLEMENTATION (THE ENGINE)
// ============================================================================

/**
 * @brief Determines which widget is at a given point (hit-testing).
 * 
 * This method checks if the point is within the widget's bounds. If so, it
 * recursively tests its children in reverse rendering order. If no child is
 * hit, this widget is the target.
 * 
 * @param point The point to test, in parent-relative coordinates.
 * @return A pointer to the top-most widget at the point, or `nullptr` if no widget is hit.
 */
IWidget* Widget::hitTest(const Point<int32_t>& point) {
    // 1. Check visibility
    if (!isVisible()) return nullptr;
    
    // 2. Check if point is inside this widget's bounds
    auto rect = getRect();
    if (point.x < rect.x || point.x >= static_cast<int32_t>(rect.getRight()) ||
        point.y < rect.y || point.y >= static_cast<int32_t>(rect.getBottom())) {
        return nullptr;
    }
    
    // 3. Test children in REVERSE order (top-most first in Z-order)
    for (auto it = pImpl_->children.rbegin(); it != pImpl_->children.rend(); ++it) {
        auto* result = (*it)->hitTest(point);
        if (result) {
            return result;  // Child (or descendant) was hit
        }
    }
    
    // 4. No child was hit, so this widget is the target
    return this;
}

// ============================================================================
// EVENT HANDLING - SIMPLIFIED (Dispatch logic moved to Window)
// ============================================================================

/**
 * @brief Virtual method for handling events.
 * 
 * Derived classes should override this method to process specific events.
 * 
 * @param event The event to handle.
 * @return `true` if the event was handled, `false` to allow further processing.
 */
bool Widget::onEvent(const event::Event& event) {
    // Default implementation: no handling
    // Derived classes override this to handle specific events
    return false;
}

// ============================================================================
// RENDERING
// ============================================================================

/**
 * @brief Renders the widget and its children.
 * 
 * This base implementation renders the widget's background color and then
 * iterates through its children, calling their render methods.
 * 
 * @param renderer The renderer to use for drawing.
 */
void Widget::render(Renderer& renderer) {
    if (!pImpl_->visible) return;

    // Render background
    renderer.fillRect(pImpl_->rect, pImpl_->backgroundColor);

    // Render children
    for (auto& child : pImpl_->children) {
        if (child->isVisible()) {
            child->render(renderer);
        }
    }
}

// ============================================================================
// HIERARCHY
// ============================================================================

/**
 * @brief Adds a child widget to this widget.
 * @param child A shared pointer to the widget to add.
 */
void Widget::addChild(std::shared_ptr<IWidget> child) {
    if (!child) return;

    if (auto* childWidget = dynamic_cast<Widget*>(child.get())) {
        if (childWidget->pImpl_->parent) {
            childWidget->pImpl_->parent->removeChild(child.get());
        }
        childWidget->pImpl_->parent = this;
        childWidget->pImpl_->windowHandle = pImpl_->getWindowHandle();
    }

    pImpl_->children.push_back(std::move(child));
    invalidate();
}

/**
 * @brief Removes a child widget from this widget.
 * @param child A pointer to the widget to remove.
 */
void Widget::removeChild(IWidget* child) {
    if (!child) return;

    auto it = std::find_if(pImpl_->children.begin(), pImpl_->children.end(),
        [child](const auto& ptr) { return ptr.get() == child; });

    if (it != pImpl_->children.end()) {
        if (auto* childWidget = dynamic_cast<Widget*>(child)) {
            childWidget->pImpl_->parent = nullptr;
            childWidget->pImpl_->windowHandle = nullptr;
        }
        pImpl_->children.erase(it);
        invalidate();
    }
}

/**
 * @brief Gets the list of child widgets.
 * @return A const reference to the vector of child widgets.
 */
const std::vector<std::shared_ptr<IWidget>>& Widget::getChildren() const noexcept {
    return pImpl_->children;
}

/**
 * @brief Gets the parent of this widget.
 * @return A pointer to the parent widget, or `nullptr` if it has no parent.
 */
IWidget* Widget::getParent() const noexcept {
    return pImpl_->parent;
}

// ============================================================================
// LAYOUT PROPERTIES
// ============================================================================

/**
 * @brief Sets the layout weight for use in a flex layout.
 * @param weight The layout weight.
 */
void Widget::setLayoutWeight(float weight) noexcept {
    if (pImpl_->layoutProps.weight == weight) return;
    pImpl_->layoutProps.weight = weight;
    
    if (pImpl_->parent) {
        if (auto* parentWidget = dynamic_cast<Widget*>(pImpl_->parent)) {
            parentWidget->invalidate();
        }
    }
}

/** @brief Gets the layout weight. @return The layout weight. */
float Widget::getLayoutWeight() const noexcept {
    return pImpl_->layoutProps.weight;
}

/** @brief Sets the minimum size for layout calculations. */
void Widget::setMinSize(int32_t width, int32_t height) noexcept {
    pImpl_->layoutProps.minWidth = width;
    pImpl_->layoutProps.minHeight = height;
    if (pImpl_->parent) {
        if (auto* parentWidget = dynamic_cast<Widget*>(pImpl_->parent)) {
            parentWidget->invalidate();
        }
    }
}

/** @brief Sets the maximum size for layout calculations. */
void Widget::setMaxSize(int32_t width, int32_t height) noexcept {
    pImpl_->layoutProps.maxWidth = width;
    pImpl_->layoutProps.maxHeight = height;
    if (pImpl_->parent) {
        if (auto* parentWidget = dynamic_cast<Widget*>(pImpl_->parent)) {
            parentWidget->invalidate();
        }
    }
}

/** @brief Sets the minimum width for layout calculations. */
void Widget::setMinWidth(int32_t width) noexcept {
    pImpl_->layoutProps.minWidth = width;
    if (pImpl_->parent) {
        if (auto* parentWidget = dynamic_cast<Widget*>(pImpl_->parent)) {
            parentWidget->invalidate();
        }
    }
}

/** @brief Sets the maximum width for layout calculations. */
void Widget::setMaxWidth(int32_t width) noexcept {
    pImpl_->layoutProps.maxWidth = width;
    if (pImpl_->parent) {
        if (auto* parentWidget = dynamic_cast<Widget*>(pImpl_->parent)) {
            parentWidget->invalidate();
        }
    }
}

/** @brief Sets the minimum height for layout calculations. */
void Widget::setMinHeight(int32_t height) noexcept {
    pImpl_->layoutProps.minHeight = height;
    if (pImpl_->parent) {
        if (auto* parentWidget = dynamic_cast<Widget*>(pImpl_->parent)) {
            parentWidget->invalidate();
        }
    }
}

/** @brief Sets the maximum height for layout calculations. */
void Widget::setMaxHeight(int32_t height) noexcept {
    pImpl_->layoutProps.maxHeight = height;
    if (pImpl_->parent) {
        if (auto* parentWidget = dynamic_cast<Widget*>(pImpl_->parent)) {
            parentWidget->invalidate();
        }
    }
}

/** @brief Sets the self-alignment property for use in a flex layout. */
void Widget::setAlignSelf(LayoutProps::Align align) noexcept {
    if (pImpl_->layoutProps.alignSelf == align) return;
    pImpl_->layoutProps.alignSelf = align;
    if (pImpl_->parent) {
        if (auto* parentWidget = dynamic_cast<Widget*>(pImpl_->parent)) {
            parentWidget->invalidate();
        }
    }
}

/** @brief Gets the self-alignment property. @return The alignment value. */
LayoutProps::Align Widget::getAlignSelf() const noexcept {
    return pImpl_->layoutProps.alignSelf;
}

/** @brief Gets a const reference to all layout properties. */
const LayoutProps& Widget::getLayoutProps() const noexcept {
    return pImpl_->layoutProps;
}

/** @brief Gets a mutable reference to all layout properties. */
LayoutProps& Widget::getLayoutPropsMut() noexcept {
    return pImpl_->layoutProps;
}

// ============================================================================
// COLOR MANAGEMENT
// ============================================================================

/**
 * @brief Sets the background color of the widget.
 * @param color The new background color.
 */
void Widget::setBackgroundColor(const Color& color) noexcept {
    if (pImpl_->backgroundColor == color) return;
    
    pImpl_->backgroundColor = color;
    invalidate();
}

/**
 * @brief Gets the background color of the widget.
 * @return The current background color.
 */
Color Widget::getBackgroundColor() const noexcept {
    return pImpl_->backgroundColor;
}

// ============================================================================
// INVALIDATION
// ============================================================================

/**
 * @brief Invalidates the entire widget area, queuing it for a repaint.
 */
void Widget::invalidate() noexcept {
    HWND hwnd = pImpl_->getWindowHandle();
    if (!hwnd) return;
    
    auto rect = pImpl_->rect;
    RECT r = {
        static_cast<LONG>(rect.x),
        static_cast<LONG>(rect.y),
        static_cast<LONG>(rect.getRight()),
        static_cast<LONG>(rect.getBottom())
    };
    
    InvalidateRect(hwnd, &r, FALSE);
}

/**
 * @brief Invalidates a specific rectangular area within the widget, queuing it for a repaint.
 * @param rect The rectangle to invalidate, in parent-relative coordinates.
 */
void Widget::invalidateRect(const Rect<int32_t, uint32_t>& rect) noexcept {
    HWND hwnd = pImpl_->getWindowHandle();
    if (!hwnd) return;
    
    RECT r = {
        static_cast<LONG>(rect.x),
        static_cast<LONG>(rect.y),
        static_cast<LONG>(rect.getRight()),
        static_cast<LONG>(rect.getBottom())
    };
    
    InvalidateRect(hwnd, &r, FALSE);
}

/**
 * @brief Internal functions for use by the framework.
 */
namespace internal {
    /**
     * @brief Sets the window handle for a widget and all its children recursively.
     * @param widget The widget to start from.
     * @param hwnd The window handle (as a void pointer).
     * @internal
     */
    void setWidgetWindowHandle(Widget* widget, void* hwnd) {
        if (!widget) return;
        widget->pImpl_->windowHandle = static_cast<HWND>(hwnd);
        
        for (auto& child : widget->pImpl_->children) {
            if (auto* childWidget = dynamic_cast<Widget*>(child.get())) {
                setWidgetWindowHandle(childWidget, hwnd);
            }
        }
    }
}

} // namespace frqs::widget
