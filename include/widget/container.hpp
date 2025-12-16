#pragma once

#include "iwidget.hpp"
#include "layout.hpp"
#include <memory>

namespace frqs::widget {

// ============================================================================
// CONTAINER WIDGET (Layout-aware parent widget)
// ============================================================================

class Container : public Widget {
private:
    std::unique_ptr<ILayout> layout_;
    uint32_t padding_ = 0;
    Color borderColor_ = colors::Transparent;
    float borderWidth_ = 0.0f;
    bool autoLayout_ = true;

public:
    Container();
    ~Container() override = default;

    // Layout management
    void setLayout(std::unique_ptr<ILayout> layout);
    ILayout* getLayout() const noexcept { return layout_.get(); }
    
    void setPadding(uint32_t padding) noexcept;
    uint32_t getPadding() const noexcept { return padding_; }

    // Border
    void setBorder(const Color& color, float width) noexcept;
    void setBorderColor(const Color& color) noexcept { borderColor_ = color; }
    void setBorderWidth(float width) noexcept { borderWidth_ = width; }

    // Auto-layout on resize
    void setAutoLayout(bool enable) noexcept { autoLayout_ = enable; }
    bool isAutoLayoutEnabled() const noexcept { return autoLayout_; }

    // Apply layout to children
    void applyLayout();

    // Override to trigger layout on rect change
    void setRect(const Rect<int32_t, uint32_t>& rect) override;

    // Rendering
    void render(Renderer& renderer) override;
};

// ============================================================================
// CONVENIENCE FUNCTIONS
// ============================================================================

// Create container with vertical stack layout
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

// Create container with horizontal stack layout
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

// Create container with grid layout
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

} // namespace frqs::widget