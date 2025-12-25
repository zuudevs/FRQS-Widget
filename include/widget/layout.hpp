/**
 * @file layout.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines various layout managers for arranging widgets within a container.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 * This file provides a set of classes for controlling the size and position of
 * child widgets within a parent widget. It includes a flexible layout system
 * (FlexLayout), a simple stack-based layout (StackLayout), a grid-based layout
 * (GridLayout), and an absolute positioning layout (AbsoluteLayout).
 */

#pragma once

#include "iwidget.hpp"
#include <vector>
#include <memory>
#include <algorithm>

namespace frqs::widget {

// ============================================================================
// LAYOUT INTERFACE
// ============================================================================

/**
 * @brief Interface for all layout managers.
 * 
 * A layout manager is responsible for calculating and setting the size and 
 * position of child widgets within a parent container.
 */
class ILayout {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~ILayout() noexcept = default;

    /**
     * @brief Applies the layout logic to the children of a parent widget.
     * @param[in] parent The parent widget whose children need to be arranged.
     */
    virtual void apply(IWidget* parent) = 0;

    /**
     * @brief Gets the minimum size required by the layout.
     * @return The minimum required size as a Size object.
     */
    virtual Size<uint32_t> getMinimumSize() const noexcept = 0;

    /**
     * @brief Gets the preferred size for the layout.
     * @return The preferred size as a Size object.
     */
    virtual Size<uint32_t> getPreferredSize() const noexcept = 0;
};

// ============================================================================
// STACK LAYOUT (Vertical or Horizontal) - LEGACY
// ============================================================================

/**
 * @brief A simple layout that arranges widgets in a single row or column.
 * @note This is considered a legacy layout. Prefer FlexLayout for more control.
 */
class StackLayout : public ILayout {
public:
    /**
     * @brief Defines the orientation of the stack.
     */
    enum class Direction : uint8_t {
        Vertical,   ///< Arrange widgets from top to bottom.
        Horizontal  ///< Arrange widgets from left to right.
    };

private:
    Direction direction_; ///< The orientation of the layout.
    uint32_t spacing_;    ///< The space between adjacent widgets.
    uint32_t padding_;    ///< The space between the container edges and the content.

public:
    /**
     * @brief Constructs a new StackLayout object.
     * @param dir The direction of the layout (Vertical or Horizontal).
     * @param spacing The space between widgets.
     * @param padding The padding around the content.
     */
    explicit StackLayout(Direction dir = Direction::Vertical,
                        uint32_t spacing = 0,
                        uint32_t padding = 0)
        : direction_(dir), spacing_(spacing), padding_(padding) {}

    /**
     * @brief Applies the stack layout logic to the parent's children.
     * @param[in] parent The container widget.
     */
    void apply(IWidget* parent) override;

    /**
     * @brief Gets the minimum size for this layout.
     * @return The minimum size.
     */
    Size<uint32_t> getMinimumSize() const noexcept override {
        return Size(100u, 100u);  // Placeholder
    }

    /**
     * @brief Gets the preferred size for this layout.
     * @return The preferred size.
     */
    Size<uint32_t> getPreferredSize() const noexcept override {
        return Size(200u, 200u);  // Placeholder
    }

    /**
     * @brief Sets the layout direction.
     * @param dir The new direction.
     */
    void setDirection(Direction dir) noexcept { direction_ = dir; }

    /**
     * @brief Sets the spacing between widgets.
     * @param spacing The new spacing value.
     */
    void setSpacing(uint32_t spacing) noexcept { spacing_ = spacing; }

    /**
     * @brief Sets the padding around the content.
     * @param padding The new padding value.
     */
    void setPadding(uint32_t padding) noexcept { padding_ = padding; }
};

// ============================================================================
// FLEX LAYOUT (Advanced weighted distribution with alignment)
// ============================================================================

/**
 * @brief An advanced layout that distributes space based on weights and alignment.
 * 
 * @details FlexLayout arranges items along a primary axis (main axis) and allows for
 * alignment along the secondary axis (cross axis). It supports flexible sizing
 * via weights (flex-grow), minimum/maximum size constraints, and per-widget
 * alignment.
 */
class FlexLayout : public ILayout {
public:
    /**
     * @brief Defines the primary axis of the layout.
     */
    enum class Direction : uint8_t {
        Row,        ///< Horizontal (main axis: left-to-right)
        Column      ///< Vertical (main axis: top-to-bottom)
    };

private:
    Direction direction_ = Direction::Row; ///< The primary axis direction.
    uint32_t gap_ = 0;                     ///< Space between items along the main axis.
    uint32_t padding_ = 0;                 ///< Container padding on all sides.

    /**
     * @brief A helper struct to store layout-related information for a child widget.
     */
    struct ChildInfo {
        IWidget* widget;        ///< Pointer to the child widget interface.
        Widget* typedWidget;    ///< Cached cast to Widget (may be null).
        LayoutProps props;      ///< The layout properties of the widget.
        int32_t allocatedSize;  ///< Calculated size along the main axis.
        bool isVisible;         ///< Visibility flag of the widget.
    };

public:
    /**
     * @brief Constructs a new FlexLayout object.
     * @param dir The primary direction of the layout (Row or Column).
     * @param gap The space between items.
     * @param padding The padding around the container.
     */
    explicit FlexLayout(Direction dir = Direction::Row,
                       uint32_t gap = 0,
                       uint32_t padding = 0)
        : direction_(dir), gap_(gap), padding_(padding) {}

    /**
     * @brief Applies the flex layout logic.
     * 
     * This method performs a three-pass calculation:
     * 1. Allocates space for fixed-size (weight=0) items.
     * 2. Distributes remaining space among flexible (weight>0) items.
     * 3. Positions all items, applying alignment properties.
     * 
     * @param[in] parent The container widget.
     */
    void apply(IWidget* parent) override;

    /**
     * @brief Gets the minimum size required by the layout.
     * @todo Calculate this based on children's minimum sizes.
     * @return The minimum size.
     */
    Size<uint32_t> getMinimumSize() const noexcept override {
        return Size(100u, 100u);  // TODO: Calculate from children
    }

    /**
     * @brief Gets the preferred size for the layout.
     * @todo Calculate this based on children's preferred sizes.
     * @return The preferred size.
     */
    Size<uint32_t> getPreferredSize() const noexcept override {
        return Size(200u, 200u);  // TODO: Calculate from children
    }

    /**
     * @brief Sets the layout direction (main axis).
     * @param dir The new direction.
     */
    void setDirection(Direction dir) noexcept { direction_ = dir; }

    /**
     * @brief Sets the gap between items.
     * @param gap The new gap value in pixels.
     */
    void setGap(uint32_t gap) noexcept { gap_ = gap; }

    /**
     * @brief Sets the padding for the container.
     * @param padding The new padding value in pixels.
     */
    void setPadding(uint32_t padding) noexcept { padding_ = padding; }
    
    /**
     * @brief Gets the current layout direction.
     * @return The layout direction.
     */
    Direction getDirection() const noexcept { return direction_; }

    /**
     * @brief Gets the current gap between items.
     * @return The gap in pixels.
     */
    uint32_t getGap() const noexcept { return gap_; }

    /**
     * @brief Gets the current padding for the container.
     * @return The padding in pixels.
     */
    uint32_t getPadding() const noexcept { return padding_; }
};

// ============================================================================
// GRID LAYOUT (Unchanged)
// ============================================================================

/**
 * @brief Arranges widgets in a grid of a specified number of rows and columns.
 */
class GridLayout : public ILayout {
private:
    uint32_t rows_;     ///< The number of rows in the grid.
    uint32_t cols_;     ///< The number of columns in the grid.
    uint32_t spacing_;  ///< The space between cells, both horizontally and vertically.
    uint32_t padding_;  ///< The padding around the entire grid.

public:
    /**
     * @brief Constructs a new GridLayout object.
     * @param rows The number of rows.
     * @param cols The number of columns.
     * @param spacing The space between cells.
     * @param padding The padding around the grid.
     */
    GridLayout(uint32_t rows, uint32_t cols,
              uint32_t spacing = 0, uint32_t padding = 0)
        : rows_(rows), cols_(cols), spacing_(spacing), padding_(padding) {}

    /**
     * @brief Applies the grid layout logic.
     * @param[in] parent The container widget.
     */
    void apply(IWidget* parent) override;

    /**
     * @brief Gets the minimum size for this layout.
     * @return The minimum size.
     */
    Size<uint32_t> getMinimumSize() const noexcept override {
        return Size(cols_ * 50u, rows_ * 50u);
    }

    /**
     * @brief Gets the preferred size for this layout.
     * @return The preferred size.
     */
    Size<uint32_t> getPreferredSize() const noexcept override {
        return Size(cols_ * 100u, rows_ * 100u);
    }
};

// ============================================================================
// ABSOLUTE LAYOUT (Manual positioning) - Unchanged
// ============================================================================

/**
 * @brief A layout that performs no automatic positioning.
 * 
 * @details Widgets in a container with an AbsoluteLayout retain the size and position
 * they were given manually via `setRect`. This is useful when widgets need to
 * be placed at explicit coordinates.
 */
class AbsoluteLayout : public ILayout {
public:
    /**
     * @brief Applies the absolute layout logic (which does nothing).
     * @param[in] parent The container widget (ignored).
     */
    void apply(IWidget* parent) override {
        // Do nothing - widgets keep their manually set positions
        (void)parent;
    }

    /**
     * @brief Gets the minimum size for this layout.
     * @return A size of (0, 0) as this layout imposes no size.
     */
    Size<uint32_t> getMinimumSize() const noexcept override {
        return Size(0u, 0u);
    }

    /**
     * @brief Gets the preferred size for this layout.
     * @return A size of (0, 0) as this layout imposes no size.
     */
    Size<uint32_t> getPreferredSize() const noexcept override {
        return Size(0u, 0u);
    }
};

} // namespace frqs::widget