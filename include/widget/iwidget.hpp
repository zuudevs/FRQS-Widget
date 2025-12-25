/**
 * @file iwidget.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines the core interfaces and base classes for the widget system.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 * This file contains the fundamental building blocks for creating UI elements:
 * - IWidget: A pure virtual interface defining the contract for all widgets.
 * - Widget: A concrete base class for widgets, implementing the IWidget interface
 *   and using the PImpl idiom to hide implementation details.
 * - LayoutProps: A POD struct for configuring widget layout properties like
 *   flex weight and alignment.
 * - Renderer: An abstract interface for drawing operations, decoupling widgets
 *   from the underlying graphics API (e.g., Direct2D).
 */
#pragma once

#include <memory>
#include <string>
#include <vector>
#include "unit/rect.hpp"
#include "unit/color.hpp"
#include "event/event.hpp"

namespace frqs::widget {

// Forward declarations
class IWidget;
class Widget;
class Renderer;

namespace internal {
    void setWidgetWindowHandle(Widget*, void*);
}

// ============================================================================
// LAYOUT PROPERTIES (POD struct - zero overhead)
// ============================================================================

/**
 * @struct LayoutProps
 * @brief Defines layout properties for a widget within a container.
 * @details This is a Plain Old Data (POD) struct, ensuring it can be used
 *          with zero overhead.
 */
struct LayoutProps {
    /**
     * @brief The flex-grow factor. If greater than 0, the widget will grow
     *        to share a proportion of the remaining space in a flex container.
     *        A weight of 0 means the widget will use its preferred size.
     */
    float weight = 0.0f;
    
    // Size constraints
    int32_t minWidth = 0;       //!< Minimum allowed width for the widget.
    int32_t maxWidth = 99999;   //!< Maximum allowed width for the widget.
    int32_t minHeight = 0;      //!< Minimum allowed height for the widget.
    int32_t maxHeight = 99999;  //!< Maximum allowed height for the widget.
    
    /**
     * @enum Align
     * @brief Defines the alignment of a widget within its allocated space.
     */
    enum class Align : uint8_t {
        Start,      //!< Align to the left (horizontal) or top (vertical).
        Center,     //!< Align to the center.
        End,        //!< Align to the right (horizontal) or bottom (vertical).
        Stretch     //!< Stretch to fill the available space (default).
    };

    /**
     * @brief The self-alignment of the widget within its parent's layout cell.
     */
    Align alignSelf = Align::Stretch;
};

// ============================================================================
// WIDGET INTERFACE (Virtual for polymorphism at high level ONLY)
// ============================================================================

/**
 * @class IWidget
 * @brief Pure virtual interface for all widgets.
 * @details Defines the essential contract that all UI elements must adhere to,
 *          enabling polymorphic behavior in the widget hierarchy.
 */
class IWidget {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IWidget() noexcept = default;

    /**
     * @brief Sets the widget's bounding rectangle.
     * @param rect The new rectangle (position and size).
     */
    virtual void setRect(const Rect<int32_t, uint32_t>& rect) = 0;

    /**
     * @brief Gets the widget's bounding rectangle.
     * @return Rect<int32_t, uint32_t> The current rectangle.
     */
    virtual Rect<int32_t, uint32_t> getRect() const noexcept = 0;

    /**
     * @brief Sets the visibility of the widget.
     * @param visible True to make the widget visible, false to hide it.
     */
    virtual void setVisible(bool visible) noexcept = 0;

    /**
     * @brief Checks if the widget is visible.
     * @return bool True if the widget is visible, false otherwise.
     */
    virtual bool isVisible() const noexcept = 0;

    /**
     * @brief Performs a hit test to find which widget is at a given point.
     * @param point The point to test, in parent coordinates.
     * @return IWidget* A pointer to the topmost widget at the given point, or nullptr if none.
     */
    virtual IWidget* hitTest(const Point<int32_t>& point) = 0;

    /**
     * @brief Handles an incoming event.
     * @param event The event to process.
     * @return bool True if the event was handled, false otherwise.
     */
    virtual bool onEvent(const event::Event& event) = 0;

    /**
     * @brief Renders the widget using the provided renderer.
     * @param renderer The renderer to use for drawing operations.
     */
    virtual void render(Renderer& renderer) = 0;

    /**
     * @brief Adds a child widget.
     * @param child A shared pointer to the child widget to add.
     */
    virtual void addChild(std::shared_ptr<IWidget> child) = 0;

    /**
     * @brief Removes a child widget.
     * @param child A pointer to the child widget to remove.
     */
    virtual void removeChild(IWidget* child) = 0;

    /**
     * @brief Gets the list of child widgets.
     * @return const std::vector<std::shared_ptr<IWidget>>& A const reference to the vector of children.
     */
    virtual const std::vector<std::shared_ptr<IWidget>>& getChildren() const noexcept = 0;

    /**
     * @brief Gets the parent widget.
     * @return IWidget* A pointer to the parent widget, or nullptr if this is a top-level widget.
     */
    virtual IWidget* getParent() const noexcept = 0;
};

// ============================================================================
// CONCRETE WIDGET BASE (PImpl Idiom)
// ============================================================================

/**
 * @class Widget
 * @brief A concrete base class for most widgets, implementing the IWidget interface.
 * @details Uses the PImpl (Pointer to Implementation) idiom to hide private
 *          data and implementation details, reducing compilation dependencies.
 */
class Widget : public IWidget {
private:
    struct Impl;  // Forward declare implementation
    std::unique_ptr<Impl> pImpl_;  // Hide implementation details

public:
    /**
     * @brief Default constructor.
     */
    explicit Widget();
    /**
     * @brief Destructor.
     */
    ~Widget() noexcept override;

    // Non-copyable (use shared_ptr for sharing)
    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;

    // Movable
    Widget(Widget&&) noexcept;
    Widget& operator=(Widget&&) noexcept;

    // IWidget interface implementation
    void setRect(const Rect<int32_t, uint32_t>& rect) override;
    Rect<int32_t, uint32_t> getRect() const noexcept override;
    void setVisible(bool visible) noexcept override;
    bool isVisible() const noexcept override;
    IWidget* hitTest(const Point<int32_t>& point) override;
    bool onEvent(const event::Event& event) override;
    void render(Renderer& renderer) override;
    void addChild(std::shared_ptr<IWidget> child) override;
    void removeChild(IWidget* child) override;
    const std::vector<std::shared_ptr<IWidget>>& getChildren() const noexcept override;
    IWidget* getParent() const noexcept override;

    /**
     * @brief Gets the widget's position. Non-virtual for performance.
     * @tparam T The coordinate type (e.g., int32_t, float).
     * @return constexpr Point<T> The top-left position.
     */
    template <typename T = int32_t>
    constexpr Point<T> getPosition() const noexcept;

    /**
     * @brief Gets the widget's size. Non-virtual for performance.
     * @tparam T The size type (e.g., uint32_t, float).
     * @return constexpr Size<T> The width and height.
     */
    template <typename T = uint32_t>
    constexpr Size<T> getSize() const noexcept;

    /**
     * @brief Sets the background color of the widget.
     * @param color The new background color.
     */
    void setBackgroundColor(const Color& color) noexcept;

    /**
     * @brief Gets the background color of the widget.
     * @return Color The current background color.
     */
    Color getBackgroundColor() const noexcept;

    /**
     * @brief Invalidates the entire widget, marking it as needing a repaint.
     */
    void invalidate() noexcept;

    /**
     * @brief Invalidates a specific rectangular area of the widget.
     * @param rect The rectangle to invalidate, in local widget coordinates.
     */
    void invalidateRect(const Rect<int32_t, uint32_t>& rect) noexcept;

    // ========================================================================
    // LAYOUT PROPERTIES
    // ========================================================================

    /**
     * @brief Sets the layout weight (flex-grow factor).
     * @param weight The new layout weight.
     */
    void setLayoutWeight(float weight) noexcept;
    float getLayoutWeight() const noexcept;

    void setMinSize(int32_t width, int32_t height) noexcept;
    void setMaxSize(int32_t width, int32_t height) noexcept;
    void setMinWidth(int32_t width) noexcept;
    void setMaxWidth(int32_t width) noexcept;
    void setMinHeight(int32_t height) noexcept;
    void setMaxHeight(int32_t height) noexcept;

    /**
     * @brief Sets the self-alignment property for layout.
     * @param align The new alignment value.
     */
    void setAlignSelf(LayoutProps::Align align) noexcept;
    LayoutProps::Align getAlignSelf() const noexcept;

    /**
     * @brief Gets a const reference to the widget's layout properties.
     * @return const LayoutProps& The layout properties.
     */
    const LayoutProps& getLayoutProps() const noexcept;

    /**
     * @brief Gets a mutable reference to the widget's layout properties.
     * @return LayoutProps& The layout properties.
     */
    LayoutProps& getLayoutPropsMut() noexcept;

    friend void internal::setWidgetWindowHandle(Widget* widget, void* hwnd);
};

// ============================================================================
// RENDERER INTERFACE (Direct2D abstraction)
// ============================================================================

/**
 * @class Renderer
 * @brief An abstract interface for rendering operations.
 * @details Decouples widgets from the specific graphics backend (e.g., Direct2D,
 *          OpenGL, Skia) by providing a common set of drawing commands.
 */
class Renderer {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~Renderer() noexcept = default;

    /**
     * @brief Clears the entire render target with a specified color.
     * @param color The color to clear with.
     */
    virtual void clear(const Color& color) = 0;

    /**
     * @brief Draws the outline of a rectangle.
     * @param rect The rectangle to draw.
     * @param color The color of the outline.
     * @param strokeWidth The thickness of the outline.
     */
    virtual void drawRect(const Rect<int32_t, uint32_t>& rect, 
                         const Color& color, 
                         float strokeWidth = 1.0f) = 0;

    /**
     * @brief Fills the interior of a rectangle.
     * @param rect The rectangle to fill.
     * @param color The color to fill with.
     */
    virtual void fillRect(const Rect<int32_t, uint32_t>& rect, 
                         const Color& color) = 0;
                         
    /**
     * @brief Draws text within a specified rectangle.
     * @param text The text to draw.
     * @param rect The bounding box for the text.
     * @param color The color of the text.
     */
    virtual void drawText(const std::wstring& text,
                         const Rect<int32_t, uint32_t>& rect,
                         const Color& color) = 0;

    /**
     * @brief Pushes a clipping rectangle onto the render stack.
     * @details All subsequent drawing operations will be clipped to this rectangle.
     * @param rect The rectangle to clip to.
     */
    virtual void pushClip(const Rect<int32_t, uint32_t>& rect) = 0;

    /**
     * @brief Pops the current clipping rectangle from the render stack.
     */
    virtual void popClip() = 0;
};

} // namespace frqs::widget