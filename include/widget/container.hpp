/**
 * @file container.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines the Container widget, a fundamental building block for layouts.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "iwidget.hpp"
#include "layout.hpp"
#include <memory>

namespace frqs::widget {

/**
 * @class Container
 * @brief A widget that holds and arranges child widgets according to a specified layout policy.
 *
 * The Container is the primary tool for building complex user interfaces. It manages a
 * collection of child widgets and uses a `ILayout` object to determine their positions
 * and sizes. It can also have its own visual properties like padding and a border.
 */
class Container : public Widget {
private:
    std::unique_ptr<ILayout> layout_;
    uint32_t padding_ = 0;
    Color borderColor_ = colors::Transparent;
    float borderWidth_ = 0.0f;
    bool autoLayout_ = true;

public:
    /**
     * @brief Constructs a new Container with a default layout.
     */
    Container();
    
    /**
     * @brief Default destructor.
     */
    ~Container() override = default;

    /**
     * @brief Sets the layout manager for this container.
     * 
     * The layout manager is responsible for arranging the container's child widgets.
     * @param layout A unique pointer to an object implementing the `ILayout` interface.
     */
    void setLayout(std::unique_ptr<ILayout> layout);

    /**
     * @brief Gets the current layout manager.
     * @return A raw pointer to the current `ILayout` object. The container retains ownership.
     */
    ILayout* getLayout() const noexcept { return layout_.get(); }
    
    /**
     * @brief Sets the internal padding for the container.
     * 
     * Padding creates space between the container's border and its contents.
     * @param padding The padding amount in pixels.
     */
    void setPadding(uint32_t padding) noexcept;
    
    /**
     * @brief Gets the internal padding of the container.
     * @return The padding amount in pixels.
     */
    uint32_t getPadding() const noexcept { return padding_; }

    /**
     * @brief Sets the color and width of the container's border.
     * @param color The color of the border.
     * @param width The width of the border in pixels.
     */
    void setBorder(const Color& color, float width) noexcept;
    
    /**
     * @brief Sets the color of the container's border.
     * @param color The new border color.
     */
    void setBorderColor(const Color& color) noexcept { borderColor_ = color; }
    
    /**
     * @brief Sets the width of the container's border.
     * @param width The new border width in pixels.
     */
    void setBorderWidth(float width) noexcept { borderWidth_ = width; }

    /**
     * @brief Enables or disables automatic layout updates when the container is resized.
     * @param enable True to enable automatic layout, false to disable.
     */
    void setAutoLayout(bool enable) noexcept { autoLayout_ = enable; }
    
    /**
     * @brief Checks if automatic layout is enabled.
     * @return True if automatic layout is enabled, false otherwise.
     */
    bool isAutoLayoutEnabled() const noexcept { return autoLayout_; }

    /**
     * @brief Manually triggers the layout to rearrange its child widgets.
     */
    void applyLayout();

    /**
     * @brief Sets the rectangle (position and size) of the widget.
     * 
     * If auto-layout is enabled, this will also trigger a layout update.
     * @param rect The new rectangle.
     */
    void setRect(const Rect<int32_t, uint32_t>& rect) override;

    /**
     * @brief Renders the container and its children.
     * @param renderer The renderer to draw with.
     */
    void render(Renderer& renderer) override;
};

// ============================================================================
// CONVENIENCE FUNCTIONS
// ============================================================================

/**
 * @brief Creates a container with a vertical `StackLayout`.
 * @param spacing The space between child widgets.
 * @param padding The internal padding of the container.
 * @return A shared pointer to the newly created Container.
 */
inline std::shared_ptr<Container> createVStack(
    uint32_t spacing = 0, 
    uint32_t padding = 0
) {
    auto container = std::make_shared<Container>();
    container->setLayout(std::make_unique<StackLayout>(
        StackLayout::Direction::Vertical, spacing, padding
    ));
    return container;
}

/**
 * @brief Creates a container with a horizontal `StackLayout`.
 * @param spacing The space between child widgets.
 * @param padding The internal padding of the container.
 * @return A shared pointer to the newly created Container.
 */
inline std::shared_ptr<Container> createHStack(
    uint32_t spacing = 0,
    uint32_t padding = 0
) {
    auto container = std::make_shared<Container>();
    container->setLayout(std::make_unique<StackLayout>(
        StackLayout::Direction::Horizontal, spacing, padding
    ));
    return container;
}

/**
 * @brief Creates a container with a `GridLayout`.
 * @param rows The number of rows in the grid.
 * @param cols The number of columns in the grid.
 * @param spacing The space between cells in the grid.
 * @param padding The internal padding of the container.
 * @return A shared pointer to the newly created Container.
 */
inline std::shared_ptr<Container> createGrid(
    uint32_t rows,
    uint32_t cols,
    uint32_t spacing = 0,
    uint32_t padding = 0
) {
    auto container = std::make_shared<Container>();
    container->setLayout(std::make_unique<GridLayout>(
        rows, cols, spacing, padding
    ));
    return container;
}

/**
 * @brief Creates a container with a row-oriented `FlexLayout`.
 * @param gap The space between child widgets.
 * @param padding The internal padding of the container.
 * @return A shared pointer to the newly created Container.
 */
inline std::shared_ptr<Container> createFlexRow(
    uint32_t gap = 0,
    uint32_t padding = 0
) {
    auto container = std::make_shared<Container>();
    container->setLayout(std::make_unique<FlexLayout>(
        FlexLayout::Direction::Row, gap, padding
    ));
    return container;
}

/**
 * @brief Creates a container with a column-oriented `FlexLayout`.
 * @param gap The space between child widgets.
 * @param padding The internal padding of the container.
 * @return A shared pointer to the newly created Container.
 */
inline std::shared_ptr<Container> createFlexColumn(
    uint32_t gap = 0,
    uint32_t padding = 0
) {
    auto container = std::make_shared<Container>();
    container->setLayout(std::make_unique<FlexLayout>(
        FlexLayout::Direction::Column, gap, padding
    ));
    return container;
}

} // namespace frqs::widget