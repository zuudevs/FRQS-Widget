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

struct LayoutProps {
    // Flex weight (0 = fixed size, > 0 = share remaining space proportionally)
    float weight = 0.0f;
    
    // Size constraints
    int32_t minWidth = 0;
    int32_t maxWidth = 99999;
    int32_t minHeight = 0;
    int32_t maxHeight = 99999;
    
    // Self-alignment within allocated space
    enum class Align : uint8_t {
        Start,      // Left (horizontal) or Top (vertical)
        Center,     // Center
        End,        // Right (horizontal) or Bottom (vertical)
        Stretch     // Fill available space (default)
    };
    Align alignSelf = Align::Stretch;
};

// ============================================================================
// WIDGET INTERFACE (Virtual for polymorphism at high level ONLY)
// ============================================================================

class IWidget {
public:
    virtual ~IWidget() noexcept = default;

    // Layout (virtual for flexibility)
    virtual void setRect(const Rect<int32_t, uint32_t>& rect) = 0;
    virtual Rect<int32_t, uint32_t> getRect() const noexcept = 0;
    virtual void setVisible(bool visible) noexcept = 0;
    virtual bool isVisible() const noexcept = 0;
    virtual IWidget* hitTest(const Point<int32_t>& point) = 0;

    // Event handling (virtual dispatch)
    virtual bool onEvent(const event::Event& event) = 0;

    // Rendering (virtual for widget type specialization)
    virtual void render(Renderer& renderer) = 0;

    // Hierarchy (virtual for proper cleanup)
    virtual void addChild(std::shared_ptr<IWidget> child) = 0;
    virtual void removeChild(IWidget* child) = 0;
    virtual const std::vector<std::shared_ptr<IWidget>>& getChildren() const noexcept = 0;
    virtual IWidget* getParent() const noexcept = 0;
};

// ============================================================================
// CONCRETE WIDGET BASE (PImpl Idiom)
// ============================================================================

class Widget : public IWidget {
private:
    struct Impl;  // Forward declare implementation
    std::unique_ptr<Impl> pImpl_;  // Hide implementation details

public:
    explicit Widget();
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

    // High-frequency accessors (non-virtual for performance)
    template <typename T = int32_t>
    constexpr Point<T> getPosition() const noexcept;

    template <typename T = uint32_t>
    constexpr Size<T> getSize() const noexcept;

    void setBackgroundColor(const Color& color) noexcept;
    Color getBackgroundColor() const noexcept;

    // Invalidate rect (mark dirty for repainting)
    void invalidate() noexcept;
    void invalidateRect(const Rect<int32_t, uint32_t>& rect) noexcept;

    // ========================================================================
    // LAYOUT PROPERTIES
    // ========================================================================

    void setLayoutWeight(float weight) noexcept;
    float getLayoutWeight() const noexcept;

    void setMinSize(int32_t width, int32_t height) noexcept;
    void setMaxSize(int32_t width, int32_t height) noexcept;
    void setMinWidth(int32_t width) noexcept;
    void setMaxWidth(int32_t width) noexcept;
    void setMinHeight(int32_t height) noexcept;
    void setMaxHeight(int32_t height) noexcept;

    void setAlignSelf(LayoutProps::Align align) noexcept;
    LayoutProps::Align getAlignSelf() const noexcept;

    const LayoutProps& getLayoutProps() const noexcept;
    LayoutProps& getLayoutPropsMut() noexcept;

    friend void internal::setWidgetWindowHandle(Widget* widget, void* hwnd);
};

// ============================================================================
// RENDERER INTERFACE (Direct2D abstraction)
// ============================================================================

class Renderer {
public:
    virtual ~Renderer() noexcept = default;

    // Drawing primitives (virtual for D2D implementation)
    virtual void clear(const Color& color) = 0;
    virtual void drawRect(const Rect<int32_t, uint32_t>& rect, 
                         const Color& color, 
                         float strokeWidth = 1.0f) = 0;
    virtual void fillRect(const Rect<int32_t, uint32_t>& rect, 
                         const Color& color) = 0;
    virtual void drawText(const std::wstring& text,
                         const Rect<int32_t, uint32_t>& rect,
                         const Color& color) = 0;

    // State management
    virtual void pushClip(const Rect<int32_t, uint32_t>& rect) = 0;
    virtual void popClip() = 0;
};

} // namespace frqs::widget