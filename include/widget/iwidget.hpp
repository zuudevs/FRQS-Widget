#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../unit/rect.hpp"
#include "../unit/color.hpp"
#include "../event/event.hpp"

namespace frqs::widget {

// Forward declarations
class IWidget;
class Widget;
class Renderer;

namespace internal {
	void setWidgetWindowHandle(Widget*, void*) ;
}

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
    bool onEvent(const event::Event& event) override;
    void render(Renderer& renderer) override;
    void addChild(std::shared_ptr<IWidget> child) override;
    void removeChild(IWidget* child) override;
    const std::vector<std::shared_ptr<IWidget>>& getChildren() const noexcept override;
    IWidget* getParent() const noexcept override;

    // High-frequency accessors (non-virtual for performance)
    // These use templates/concepts to avoid vtable lookup on hot path
    template <typename T = int32_t>
    constexpr Point<T> getPosition() const noexcept;

    template <typename T = uint32_t>
    constexpr Size<T> getSize() const noexcept;

    void setBackgroundColor(const Color& color) noexcept;
    Color getBackgroundColor() const noexcept;

    // Invalidate rect (mark dirty for repainting)
    void invalidate() noexcept;
    void invalidateRect(const Rect<int32_t, uint32_t>& rect) noexcept;

	friend void internal::setWidgetWindowHandle(Widget* widget, void* hwnd) ;
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