#pragma once

#include "iwidget.hpp"
#include <vector>
#include <memory>

namespace frqs::widget {

// ============================================================================
// LAYOUT INTERFACE
// ============================================================================

class ILayout {
public:
    virtual ~ILayout() noexcept = default;

    // Calculate and apply layout to children
    virtual void apply(IWidget* parent) = 0;

    // Get minimum size requirements
    virtual Size<uint32_t> getMinimumSize() const noexcept = 0;

    // Get preferred size
    virtual Size<uint32_t> getPreferredSize() const noexcept = 0;
};

// ============================================================================
// STACK LAYOUT (Vertical or Horizontal)
// ============================================================================

class StackLayout : public ILayout {
public:
    enum class Direction : uint8_t {
        Vertical,
        Horizontal
    };

private:
    Direction direction_ = Direction::Vertical;
    uint32_t spacing_ = 0;
    uint32_t padding_ = 0;

public:
    explicit StackLayout(Direction dir = Direction::Vertical,
                        uint32_t spacing = 0,
                        uint32_t padding = 0)
        : direction_(dir), spacing_(spacing), padding_(padding) {}

    void apply(IWidget* parent) override {
        if (!parent) return;

        auto& children = parent->getChildren();
        if (children.empty()) return;

        auto parentRect = parent->getRect();
        int32_t x = parentRect.x + static_cast<int32_t>(padding_);
        int32_t y = parentRect.y + static_cast<int32_t>(padding_);

        uint32_t availableWidth = parentRect.w - (2 * padding_);
        uint32_t availableHeight = parentRect.h - (2 * padding_);

        if (direction_ == Direction::Vertical) {
            // Vertical stack
            uint32_t remainingHeight = availableHeight;
            size_t visibleCount = 0;

            // Count visible children
            for (const auto& child : children) {
                if (child->isVisible()) ++visibleCount;
            }

            if (visibleCount == 0) return;

            // Calculate spacing
            uint32_t totalSpacing = spacing_ * (visibleCount - 1);
            if (totalSpacing > remainingHeight) totalSpacing = 0;
            remainingHeight -= totalSpacing;

            // Layout children
            uint32_t heightPerChild = remainingHeight / static_cast<uint32_t>(visibleCount);

            for (const auto& child : children) {
                if (!child->isVisible()) continue;

                child->setRect(Rect(
                    x, y,
                    availableWidth,
                    heightPerChild
                ));

                y += static_cast<int32_t>(heightPerChild + spacing_);
            }
        } else {
            // Horizontal stack
            uint32_t remainingWidth = availableWidth;
            size_t visibleCount = 0;

            for (const auto& child : children) {
                if (child->isVisible()) ++visibleCount;
            }

            if (visibleCount == 0) return;

            uint32_t totalSpacing = spacing_ * (visibleCount - 1);
            if (totalSpacing > remainingWidth) totalSpacing = 0;
            remainingWidth -= totalSpacing;

            uint32_t widthPerChild = remainingWidth / static_cast<uint32_t>(visibleCount);

            for (const auto& child : children) {
                if (!child->isVisible()) continue;

                child->setRect(Rect(
                    x, y,
                    widthPerChild,
                    availableHeight
                ));

                x += static_cast<int32_t>(widthPerChild + spacing_);
            }
        }
    }

    Size<uint32_t> getMinimumSize() const noexcept override {
        return Size(100u, 100u);  // Placeholder
    }

    Size<uint32_t> getPreferredSize() const noexcept override {
        return Size(200u, 200u);  // Placeholder
    }

    void setDirection(Direction dir) noexcept { direction_ = dir; }
    void setSpacing(uint32_t spacing) noexcept { spacing_ = spacing; }
    void setPadding(uint32_t padding) noexcept { padding_ = padding; }
};

// ============================================================================
// GRID LAYOUT
// ============================================================================

class GridLayout : public ILayout {
private:
    uint32_t rows_ = 1;
    uint32_t cols_ = 1;
    uint32_t spacing_ = 0;
    uint32_t padding_ = 0;

public:
    GridLayout(uint32_t rows, uint32_t cols,
              uint32_t spacing = 0, uint32_t padding = 0)
        : rows_(rows), cols_(cols), spacing_(spacing), padding_(padding) {}

    void apply(IWidget* parent) override {
        if (!parent) return;

        auto& children = parent->getChildren();
        if (children.empty()) return;

        auto parentRect = parent->getRect();
        uint32_t availableWidth = parentRect.w - (2 * padding_);
        uint32_t availableHeight = parentRect.h - (2 * padding_);

        uint32_t totalHSpacing = spacing_ * (cols_ - 1);
        uint32_t totalVSpacing = spacing_ * (rows_ - 1);

        uint32_t cellWidth = (availableWidth - totalHSpacing) / cols_;
        uint32_t cellHeight = (availableHeight - totalVSpacing) / rows_;

        size_t index = 0;
        for (const auto& child : children) {
            if (!child->isVisible()) continue;
            if (index >= rows_ * cols_) break;

            uint32_t row = static_cast<uint32_t>(index) / cols_;
            uint32_t col = static_cast<uint32_t>(index) % cols_;

            int32_t x = parentRect.x + static_cast<int32_t>(
                padding_ + col * (cellWidth + spacing_)
            );
            int32_t y = parentRect.y + static_cast<int32_t>(
                padding_ + row * (cellHeight + spacing_)
            );

            child->setRect(Rect(x, y, cellWidth, cellHeight));
            ++index;
        }
    }

    Size<uint32_t> getMinimumSize() const noexcept override {
        return Size(cols_ * 50u, rows_ * 50u);
    }

    Size<uint32_t> getPreferredSize() const noexcept override {
        return Size(cols_ * 100u, rows_ * 100u);
    }
};

// ============================================================================
// ABSOLUTE LAYOUT (Manual positioning)
// ============================================================================

class AbsoluteLayout : public ILayout {
public:
    void apply(IWidget* parent) override {
        // Do nothing - widgets keep their manually set positions
        (void)parent;
    }

    Size<uint32_t> getMinimumSize() const noexcept override {
        return Size(0u, 0u);
    }

    Size<uint32_t> getPreferredSize() const noexcept override {
        return Size(0u, 0u);
    }
};

} // namespace frqs::widget