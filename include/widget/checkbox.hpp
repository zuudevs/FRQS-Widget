/**
 * @file checkbox.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines the CheckBox widget.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "iwidget.hpp"
#include <functional>
#include "render/renderer.hpp"

namespace frqs::widget {

/**
 * @class CheckBox
 * @brief A standard check box widget that allows the user to make a binary choice.
 *
 * The CheckBox widget consists of a small box and a label. The box can be either
 * checked or unchecked. It supports text labels, customizable colors, and
 * a callback for when its state changes.
 */
class CheckBox : public Widget {
public:
    /**
     * @brief Callback function type for when the checked state changes.
     * @param checked True if the box is now checked, false otherwise.
     */
    using ChangedCallback = std::function<void(bool)>;

    /**
     * @brief Defines the visual and interactive state of the checkbox.
     */
    enum class State : uint8_t {
        Normal,   ///< The default, inactive state.
        Hovered,  ///< The mouse is over the widget.
        Pressed,  ///< The widget is being clicked.
        Disabled  ///< The widget is disabled and cannot be interacted with.
    };

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;

    // State
    bool checked_ = false;
    State state_ = State::Normal;
    bool enabled_ = true;
    
    // Text
    std::wstring text_;
    Color textColor_ = colors::Black;
    render::FontStyle font_;
    
    // Visual properties
    float boxSize_ = 18.0f;
    float spacing_ = 8.0f;
    float borderRadius_ = 3.0f;
    
    // Colors
    Color boxColor_ = colors::White;
    Color boxBorderColor_ = Color(189, 195, 199);
    Color checkedColor_ = Color(52, 152, 219);
    Color hoverBorderColor_ = Color(52, 152, 219);
    Color disabledColor_ = Color(189, 195, 199);
    
    // Callback
    ChangedCallback onChanged_;

public:
    /**
     * @brief Constructs a new CheckBox.
     * @param text The initial text label for the checkbox.
     */
    explicit CheckBox(const std::wstring& text = L"");
    
    /**
     * @brief Destructor.
     */
    ~CheckBox() override;

    /**
     * @brief Sets the checked state of the checkbox.
     * @param checked True to check the box, false to uncheck it.
     */
    void setChecked(bool checked);
    
    /**
     * @brief Gets the current checked state.
     * @return True if the box is checked, false otherwise.
     */
    bool isChecked() const noexcept { return checked_; }

    /**
     * @brief Toggles the checked state of the checkbox.
     */
    void toggle() { setChecked(!checked_); }

    /**
     * @brief Enables or disables the checkbox.
     * @param enabled True to enable, false to disable.
     */
    void setEnabled(bool enabled) noexcept;

    /**
     * @brief Checks if the checkbox is enabled.
     * @return True if enabled, false otherwise.
     */
    bool isEnabled() const noexcept { return enabled_; }

    /**
     * @brief Sets the text label of the checkbox.
     * @param text The new text label.
     */
    void setText(const std::wstring& text);

    /**
     * @brief Gets the current text label.
     * @return A const reference to the text label.
     */
    const std::wstring& getText() const noexcept { return text_; }

    /**
     * @brief Sets the color of the text label.
     * @param color The new text color.
     */
    void setTextColor(const Color& color) noexcept { textColor_ = color; }

    /**
     * @brief Gets the color of the text label.
     * @return The current text color.
     */
    Color getTextColor() const noexcept { return textColor_; }

    /**
     * @brief Sets the font style for the text label.
     * @param font The new font style.
     */
    void setFont(const render::FontStyle& font) noexcept { font_ = font; }

    /**
     * @brief Sets the font size for the text label.
     * @param size The new font size.
     */
    void setFontSize(float size) noexcept { font_.size = size; }

    /**
     * @brief Sets the size of the check box square.
     * @param size The new size in pixels.
     */
    void setBoxSize(float size) noexcept { boxSize_ = size; }

    /**
     * @brief Sets the spacing between the check box and the text label.
     * @param spacing The new spacing in pixels.
     */
    void setSpacing(float spacing) noexcept { spacing_ = spacing; }

    /**
     * @brief Sets the border radius for the corners of the check box.
     * @param radius The new border radius.
     */
    void setBorderRadius(float radius) noexcept { borderRadius_ = radius; }

    /**
     * @brief Sets the background color of the check box when unchecked.
     * @param color The new color.
     */
    void setBoxColor(const Color& color) noexcept { boxColor_ = color; }

    /**
     * @brief Sets the background color of the check box when checked.
     * @param color The new color.
     */
    void setCheckedColor(const Color& color) noexcept { checkedColor_ = color; }

    /**
     * @brief Sets the border color when the mouse is hovering over the check box.
     * @param color The new color.
     */
    void setHoverBorderColor(const Color& color) noexcept { hoverBorderColor_ = color; }

    /**
     * @brief Sets the callback function to be invoked when the checked state changes.
     * @param callback The function to call.
     */
    void setOnChanged(ChangedCallback callback) { onChanged_ = std::move(callback); }

    /**
     * @brief Handles incoming events for the widget.
     * @param event The event to process.
     * @return True if the event was handled, false otherwise.
     */
    bool onEvent(const event::Event& event) override;

    /**
     * @brief Renders the checkbox using the provided renderer.
     * @param renderer The renderer to draw with.
     */
    void render(Renderer& renderer) override;

private:
    void setState(State state) noexcept;
    Color getCurrentBorderColor() const noexcept;
    bool isPointInside(const Point<int32_t>& point) const noexcept;
    void notifyChanged();
};

} // namespace frqs::widget