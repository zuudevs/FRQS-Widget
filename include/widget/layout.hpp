#pragma once

#include "iwidget.hpp"
#include <vector>
#include <memory>
#include <algorithm>

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
// STACK LAYOUT (Vertical or Horizontal) - LEGACY
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
// FLEX LAYOUT (Advanced weighted distribution with alignment)
// ============================================================================

class FlexLayout : public ILayout {
public:
    enum class Direction : uint8_t {
        Row,        // Horizontal (main axis: left-to-right)
        Column      // Vertical (main axis: top-to-bottom)
    };

private:
    Direction direction_ = Direction::Row;
    uint32_t gap_ = 0;          // Space between items
    uint32_t padding_ = 0;      // Container padding

    // Helper struct for layout calculation
    struct ChildInfo {
        IWidget* widget;
        Widget* typedWidget;    // Cached cast (may be null for non-Widget children)
        LayoutProps props;
        int32_t allocatedSize;  // Size along main axis
        bool isVisible;
    };

public:
    explicit FlexLayout(Direction dir = Direction::Row,
                       uint32_t gap = 0,
                       uint32_t padding = 0)
        : direction_(dir), gap_(gap), padding_(padding) {}

    void apply(IWidget* parent) override {
        if (!parent) return;

        auto& children = parent->getChildren();
        if (children.empty()) return;

        auto parentRect = parent->getRect();
        
        // Calculate available space
        int32_t contentStart;
        uint32_t availableMainAxis;
        uint32_t availableCrossAxis;
        
        if (direction_ == Direction::Row) {
            contentStart = parentRect.x + static_cast<int32_t>(padding_);
            availableMainAxis = parentRect.w > padding_ * 2 ? parentRect.w - padding_ * 2 : 0;
            availableCrossAxis = parentRect.h > padding_ * 2 ? parentRect.h - padding_ * 2 : 0;
        } else {
            contentStart = parentRect.y + static_cast<int32_t>(padding_);
            availableMainAxis = parentRect.h > padding_ * 2 ? parentRect.h - padding_ * 2 : 0;
            availableCrossAxis = parentRect.w > padding_ * 2 ? parentRect.w - padding_ * 2 : 0;
        }

        // Gather child info
        std::vector<ChildInfo> childInfos;
        childInfos.reserve(children.size());
        
        size_t visibleCount = 0;
        for (auto& child : children) {
            if (!child->isVisible()) continue;
            
            auto* typedWidget = dynamic_cast<Widget*>(child.get());
            ChildInfo info{
                .widget = child.get(),
                .typedWidget = typedWidget,
                .props = typedWidget ? typedWidget->getLayoutProps() : LayoutProps{},
                .allocatedSize = 0,
                .isVisible = true
            };
            childInfos.push_back(info);
            ++visibleCount;
        }

        if (visibleCount == 0) return;

        // Calculate total gap space
        uint32_t totalGapSpace = gap_ * (visibleCount - 1);
        if (totalGapSpace > availableMainAxis) totalGapSpace = 0;
        
        int32_t remainingSpace = static_cast<int32_t>(availableMainAxis - totalGapSpace);

        // ====================================================================
        // PASS 1: Allocate space for fixed-size widgets (weight == 0)
        // ====================================================================
        
        for (auto& info : childInfos) {
            if (info.props.weight <= 0.0f) {
                // Fixed size widget - use current size along main axis
                auto rect = info.widget->getRect();
                int32_t fixedSize;
                
                if (direction_ == Direction::Row) {
                    fixedSize = static_cast<int32_t>(rect.w);
                    // Apply constraints
                    fixedSize = std::clamp(fixedSize, info.props.minWidth, info.props.maxWidth);
                } else {
                    fixedSize = static_cast<int32_t>(rect.h);
                    fixedSize = std::clamp(fixedSize, info.props.minHeight, info.props.maxHeight);
                }
                
                info.allocatedSize = fixedSize;
                remainingSpace -= fixedSize;
            }
        }

        // Ensure non-negative remaining space
        if (remainingSpace < 0) remainingSpace = 0;

        // ====================================================================
        // PASS 2: Distribute remaining space to flex widgets (weight > 0)
        // ====================================================================
        
        // Calculate total weight
        float totalWeight = 0.0f;
        for (const auto& info : childInfos) {
            if (info.props.weight > 0.0f) {
                totalWeight += info.props.weight;
            }
        }

        if (totalWeight > 0.0f && remainingSpace > 0) {
            for (auto& info : childInfos) {
                if (info.props.weight > 0.0f) {
                    // Calculate proportional share
                    float share = info.props.weight / totalWeight;
                    int32_t flexSize = static_cast<int32_t>(remainingSpace * share);
                    
                    // Apply constraints
                    if (direction_ == Direction::Row) {
                        flexSize = std::clamp(flexSize, info.props.minWidth, info.props.maxWidth);
                    } else {
                        flexSize = std::clamp(flexSize, info.props.minHeight, info.props.maxHeight);
                    }
                    
                    info.allocatedSize = flexSize;
                }
            }
        }

        // ====================================================================
        // PASS 3: Position widgets with alignment
        // ====================================================================
        
        int32_t currentPos = contentStart;
        
        for (auto& info : childInfos) {
            int32_t mainAxisPos = currentPos;
            uint32_t mainAxisSize = static_cast<uint32_t>(std::max(0, info.allocatedSize));
            
            // Calculate cross-axis position based on alignment
            int32_t crossAxisPos;
            uint32_t crossAxisSize;
            
            if (direction_ == Direction::Row) {
                // Main axis: horizontal, Cross axis: vertical
                crossAxisPos = parentRect.y + static_cast<int32_t>(padding_);
                
                switch (info.props.alignSelf) {
                    case LayoutProps::Align::Start:
                        crossAxisSize = std::min(availableCrossAxis / 2, availableCrossAxis);
                        break;
                    case LayoutProps::Align::Center:
                        crossAxisSize = std::min(availableCrossAxis / 2, availableCrossAxis);
                        crossAxisPos += static_cast<int32_t>((availableCrossAxis - crossAxisSize) / 2);
                        break;
                    case LayoutProps::Align::End:
                        crossAxisSize = std::min(availableCrossAxis / 2, availableCrossAxis);
                        crossAxisPos += static_cast<int32_t>(availableCrossAxis - crossAxisSize);
                        break;
                    case LayoutProps::Align::Stretch:
                    default:
                        crossAxisSize = availableCrossAxis;
                        break;
                }
                
                // Apply cross-axis constraints
                if (info.typedWidget) {
                    crossAxisSize = std::clamp(
                        static_cast<int32_t>(crossAxisSize),
                        info.props.minHeight,
                        info.props.maxHeight
                    );
                }
                
                info.widget->setRect(Rect(mainAxisPos, crossAxisPos, mainAxisSize, crossAxisSize));
                
            } else {
                // Main axis: vertical, Cross axis: horizontal
                crossAxisPos = parentRect.x + static_cast<int32_t>(padding_);
                
                switch (info.props.alignSelf) {
                    case LayoutProps::Align::Start:
                        crossAxisSize = std::min(availableCrossAxis / 2, availableCrossAxis);
                        break;
                    case LayoutProps::Align::Center:
                        crossAxisSize = std::min(availableCrossAxis / 2, availableCrossAxis);
                        crossAxisPos += static_cast<int32_t>((availableCrossAxis - crossAxisSize) / 2);
                        break;
                    case LayoutProps::Align::End:
                        crossAxisSize = std::min(availableCrossAxis / 2, availableCrossAxis);
                        crossAxisPos += static_cast<int32_t>(availableCrossAxis - crossAxisSize);
                        break;
                    case LayoutProps::Align::Stretch:
                    default:
                        crossAxisSize = availableCrossAxis;
                        break;
                }
                
                // Apply cross-axis constraints
                if (info.typedWidget) {
                    crossAxisSize = std::clamp(
                        static_cast<int32_t>(crossAxisSize),
                        info.props.minWidth,
                        info.props.maxWidth
                    );
                }
                
                info.widget->setRect(Rect(crossAxisPos, mainAxisPos, crossAxisSize, mainAxisSize));
            }
            
            // Advance position
            currentPos += info.allocatedSize + static_cast<int32_t>(gap_);
        }
    }

    Size<uint32_t> getMinimumSize() const noexcept override {
        return Size(100u, 100u);  // TODO: Calculate from children
    }

    Size<uint32_t> getPreferredSize() const noexcept override {
        return Size(200u, 200u);  // TODO: Calculate from children
    }

    // Configuration
    void setDirection(Direction dir) noexcept { direction_ = dir; }
    void setGap(uint32_t gap) noexcept { gap_ = gap; }
    void setPadding(uint32_t padding) noexcept { padding_ = padding; }
    
    Direction getDirection() const noexcept { return direction_; }
    uint32_t getGap() const noexcept { return gap_; }
    uint32_t getPadding() const noexcept { return padding_; }
};

// ============================================================================
// GRID LAYOUT (Unchanged)
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
// ABSOLUTE LAYOUT (Manual positioning) - Unchanged
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