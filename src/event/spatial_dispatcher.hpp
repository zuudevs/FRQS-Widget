// src/event/spatial_dispatcher.hpp
#pragma once

#include <vector>
#include <memory>
#include <algorithm>

namespace frqs::event {

// Forward declarations
template <typename T, typename P, typename S>
class Rect;

template <typename T>
struct Point;

// ============================================================================
// SPATIAL HASH (Simple & Fast for UI)
// ============================================================================
// For typical UI layouts, a simple spatial hash is faster than QuadTree
// because widgets rarely overlap and are usually grid-aligned.

template <typename Widget>
class SpatialHash {
private:
    struct Cell {
        std::vector<Widget*> widgets;
    };
    
    static constexpr size_t CELL_SIZE = 64;  // 64x64 pixel cells
    
    std::vector<Cell> cells_;
    size_t gridWidth_;
    size_t gridHeight_;
    size_t areaWidth_;
    size_t areaHeight_;
    
public:
    SpatialHash(size_t width, size_t height) 
        : areaWidth_(width)
        , areaHeight_(height)
    {
        gridWidth_ = (width + CELL_SIZE - 1) / CELL_SIZE;
        gridHeight_ = (height + CELL_SIZE - 1) / CELL_SIZE;
        cells_.resize(gridWidth_ * gridHeight_);
    }
    
    // ========================================================================
    // INDEXING
    // ========================================================================
    
    size_t getCellIndex(int32_t x, int32_t y) const noexcept {
        if (x < 0 || y < 0) return size_t(-1);
        
        size_t cx = static_cast<size_t>(x) / CELL_SIZE;
        size_t cy = static_cast<size_t>(y) / CELL_SIZE;
        
        if (cx >= gridWidth_ || cy >= gridHeight_) return size_t(-1);
        
        return cy * gridWidth_ + cx;
    }
    
    // ========================================================================
    // INSERT/REMOVE
    // ========================================================================
    
    void insert(Widget* widget) {
        if (!widget) return;
        
        auto rect = widget->getRect();
        
        // Calculate bounding cells
        size_t minX = std::max<int32_t>(0, rect.x) / CELL_SIZE;
        size_t minY = std::max<int32_t>(0, rect.y) / CELL_SIZE;
        size_t maxX = std::min<size_t>((rect.x + rect.w) / CELL_SIZE, gridWidth_ - 1);
        size_t maxY = std::min<size_t>((rect.y + rect.h) / CELL_SIZE, gridHeight_ - 1);
        
        // Insert into all overlapping cells
        for (size_t cy = minY; cy <= maxY; ++cy) {
            for (size_t cx = minX; cx <= maxX; ++cx) {
                size_t idx = cy * gridWidth_ + cx;
                cells_[idx].widgets.push_back(widget);
            }
        }
    }
    
    void clear() noexcept {
        for (auto& cell : cells_) {
            cell.widgets.clear();
        }
    }
    
    // ========================================================================
    // QUERY
    // ========================================================================
    
    Widget* findWidgetAt(int32_t x, int32_t y) const {
        size_t idx = getCellIndex(x, y);
        if (idx == size_t(-1)) return nullptr;
        
        const auto& cell = cells_[idx];
        
        // Test widgets in reverse order (top to bottom Z-order)
        for (auto it = cell.widgets.rbegin(); it != cell.widgets.rend(); ++it) {
            Widget* widget = *it;
            
            if (!widget->isVisible()) continue;
            
            auto rect = widget->getRect();
            
            // AABB test
            if (x >= rect.x && x < rect.x + static_cast<int32_t>(rect.w) &&
                y >= rect.y && y < rect.y + static_cast<int32_t>(rect.h)) {
                return widget;
            }
        }
        
        return nullptr;
    }
    
    // Get all widgets in a rectangular region
    std::vector<Widget*> queryRegion(const auto& rect) const {
        std::vector<Widget*> result;
        result.reserve(16);  // Typical case
        
        size_t minX = std::max<int32_t>(0, rect.x) / CELL_SIZE;
        size_t minY = std::max<int32_t>(0, rect.y) / CELL_SIZE;
        size_t maxX = std::min<size_t>((rect.x + rect.w) / CELL_SIZE, gridWidth_ - 1);
        size_t maxY = std::min<size_t>((rect.y + rect.h) / CELL_SIZE, gridHeight_ - 1);
        
        for (size_t cy = minY; cy <= maxY; ++cy) {
            for (size_t cx = minX; cx <= maxX; ++cx) {
                size_t idx = cy * gridWidth_ + cx;
                const auto& cell = cells_[idx];
                
                for (Widget* widget : cell.widgets) {
                    // Avoid duplicates
                    if (std::find(result.begin(), result.end(), widget) == result.end()) {
                        auto widgetRect = widget->getRect();
                        
                        // Check intersection
                        if (rectsIntersect(rect, widgetRect)) {
                            result.push_back(widget);
                        }
                    }
                }
            }
        }
        
        return result;
    }
    
private:
    template <typename R>
    static bool rectsIntersect(const R& a, const R& b) noexcept {
        return !(a.x + a.w <= b.x || 
                 b.x + b.w <= a.x ||
                 a.y + a.h <= b.y || 
                 b.y + b.h <= a.y);
    }
};

// ============================================================================
// USAGE IN EVENT DISPATCHER
// ============================================================================

/*

class SpatialEventDispatcher {
private:
    SpatialHash<IWidget> spatialIndex_;
    
public:
    explicit SpatialEventDispatcher(size_t width, size_t height)
        : spatialIndex_(width, height) {}
    
    void rebuildIndex(IWidget* root) {
        spatialIndex_.clear();
        
        // Recursively add all widgets
        std::function<void(IWidget*)> addWidget = [&](IWidget* widget) {
            if (!widget || !widget->isVisible()) return;
            
            spatialIndex_.insert(widget);
            
            for (auto& child : widget->getChildren()) {
                addWidget(child.get());
            }
        };
        
        addWidget(root);
    }
    
    bool dispatchMouseEvent(const MouseEvent& evt, IWidget* root) {
        // Fast hit test using spatial hash
        IWidget* target = spatialIndex_.findWidgetAt(evt.position.x, evt.position.y);
        
        if (target) {
            return target->onEvent(Event(evt));
        }
        
        return false;
    }
};

*/

} // namespace frqs::event