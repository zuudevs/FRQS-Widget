/**
 * @file button.hpp
 * @brief Defines the Button widget, a core interactive element for the FRQS Widget toolkit.
 */

#pragma once

#include "iwidget.hpp"
#include "render/renderer.hpp"
#include <functional>

namespace frqs::widget {

/**
 * @brief A classic clickable button widget with multiple visual states.
 *
 * The Button class provides a standard push-button that can display text
 * and trigger an action when clicked. It manages visual states for normal,
 * hovered, pressed, and disabled to give user feedback.
 */
class Button : public Widget {
public:
    /**
     * @brief Defines the function signature for the button's click event handler.
     */
    using ClickCallback = std::function<void()>;

    /**
     * @brief Enumerates the possible visual states of the button.
     */
    enum class State : uint8_t {
        Normal,  ///< The default, idle state.
        Hovered, ///< The state when the mouse cursor is over the button.
        Pressed, ///< The state when the user is actively clicking the button.
        Disabled ///< The state when the button is not interactive.
    };

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;

    // Visual state
    State state_ = State::Normal;
    
    // Text
    std::wstring text_;
    Color textColor_ = colors::White;
    render::FontStyle font_;
    
    // Colors per state
    Color normalColor_ = Color(52, 152, 219);     // Blue
    Color hoverColor_ = Color(41, 128, 185);      // Darker blue
    Color pressedColor_ = Color(21, 101, 192);    // Even darker
    Color disabledColor_ = Color(149, 165, 166);  // Gray
    
    // Border
    float borderRadius_ = 4.0f;
    Color borderColor_ = colors::Transparent;
    float borderWidth_ = 0.0f;
    
    // Callback
    ClickCallback onClick_;

public:
    /**
     * @brief Constructs a new Button object.
     * @param text The initial text to display on the button.
     */
    explicit Button(const std::wstring& text = L"Button");
    ~Button() override;

    /**
     * @brief Sets the text to be displayed on the button.
     * @param text The new text for the button.
     */
    void setText(std::wstring_view text);

    /**
     * @brief Gets the current text displayed on the button.
     * @return A constant reference to the button's text.
     */
    const std::wstring& getText() const noexcept { return text_; }

    /**
     * @brief Sets the color of the button's text.
     * @param color The new text color.
     */
    void setTextColor(const Color& color) noexcept { textColor_ = color; }

    /**
     * @brief Gets the current color of the button's text.
     * @return The current text color.
     */
    Color getTextColor() const noexcept { return textColor_; }

    /**
     * @brief Sets the font style for the button's text.
     * @param font The font style attributes.
     */
    void setFont(const render::FontStyle& font) noexcept { font_ = font; }
    
    /**
     * @brief Sets the font size for the button's text.
     * @param size The new font size.
     */
    void setFontSize(float size) noexcept { font_.size = size; }

    /** @brief Sets the background color for the button's normal (idle) state. */
    void setNormalColor(const Color& color) noexcept { normalColor_ = color; }

    /** @brief Sets the background color for the button's hovered (mouse over) state. */
    void setHoverColor(const Color& color) noexcept { hoverColor_ = color; }

    /** @brief Sets the background color for the button's pressed (mouse down) state. */
    void setPressedColor(const Color& color) noexcept { pressedColor_ = color; }

    /** @brief Sets the background color for the button's disabled state. */
    void setDisabledColor(const Color& color) noexcept { disabledColor_ = color; }

    /**
     * @brief Sets the corner radius for the button's border.
     * @param radius The radius for the rounded corners.
     */
    void setBorderRadius(float radius) noexcept { borderRadius_ = radius; }
    
    /**
     * @brief Sets the color and width of the button's border.
     * @param color The color of the border.
     * @param width The width of the border.
     */
    void setBorder(const Color& color, float width) noexcept;

    /**
     * @brief Manually sets the visual state of the button.
     * @param state The new state to apply.
     */
    void setState(State state) noexcept;

    /**
     * @brief Gets the current visual state of the button.
     * @return The current button state.
     */
    State getState() const noexcept { return state_; }
    
    /**
     * @brief Enables or disables the button.
     * 
     * A disabled button does not respond to user input and is drawn in its disabled state.
     * @param enabled True to enable the button, false to disable it.
     */
    void setEnabled(bool enabled) noexcept;

    /**
     * @brief Checks if the button is currently enabled.
     * @return True if the button is enabled, false otherwise.
     */
    bool isEnabled() const noexcept { return state_ != State::Disabled; }

    /**
     * @brief Sets the callback function to be invoked when the button is clicked.
     * @param callback The function to call on a click event.
     */
    void setOnClick(ClickCallback callback) { onClick_ = std::move(callback); }

    /**
     * @brief Processes an incoming event.
     * @param event The event to process.
     * @return True if the event was handled, false otherwise.
     */
    bool onEvent(const event::Event& event) override;

    /**
     * @brief Renders the button using the provided renderer.
     * @param renderer The renderer to draw the button with.
     */
    void render(Renderer& renderer) override;

private:
    /**
     * @brief Gets the background color corresponding to the current button state.
     * @internal
     * @return The color for the current state.
     */
    Color getCurrentColor() const noexcept;

    /**
     * @brief Checks if a given point is within the button's boundaries.
     * @internal
     * @param point The point to check.
     * @return True if the point is inside the button, false otherwise.
     */
    bool isPointInside(const Point<int32_t>& point) const noexcept;
};

} // namespace frqs::widget