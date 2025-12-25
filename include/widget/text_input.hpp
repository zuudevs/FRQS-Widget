/**
 * @file text_input.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines the TextInput widget, a single-line text entry field.
 * @version 0.1.0
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "iwidget.hpp"
#include "render/renderer.hpp"
#include <functional>

namespace frqs::widget {

// ============================================================================
// TEXT INPUT WIDGET (Single-line text input with cursor and selection)
// ============================================================================

/**
 * @brief A widget for single-line text input from the user.
 * @details This widget provides a complete text entry field with support for a blinking cursor,
 * text selection (via mouse or keyboard), placeholder text, focus management,
 * and system clipboard integration (copy, cut, paste).
 */
class TextInput : public Widget {
public:
    /**
     * @brief Callback function type invoked whenever the text content changes.
     * @param text The new text content of the input field.
     */
    using TextChangedCallback = std::function<void(const std::wstring&)>;
    /**
     * @brief Callback function type invoked when the user presses the Enter/Return key.
     * @param text The current text content of the input field.
     */
    using EnterCallback = std::function<void(const std::wstring&)>;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;

    // Text content
    std::wstring text_;             ///< The current text content of the widget.
    std::wstring placeholder_ = L"Type here..."; ///< The placeholder text shown when the input is empty.
    
    // Cursor and selection
    size_t cursorPos_ = 0;          ///< The current position of the editing cursor (caret).
    size_t selectionStart_ = 0;     ///< The starting position of the text selection.
    bool hasSelection_ = false;     ///< True if there is an active text selection.
    
    // State
    bool focused_ = false;          ///< True if the widget currently has keyboard focus.
    bool enabled_ = true;           ///< If false, the widget is non-interactive.
    bool cursorVisible_ = true;     ///< The current visibility state of the blinking cursor.
    
    // Styling
    Color textColor_ = colors::Black;               ///< The color of the input text.
    Color placeholderColor_ = Color(149, 165, 166); ///< The color of the placeholder text.
    Color backgroundColor_ = colors::White;         ///< The background color of the input field.
    Color focusColor_ = Color(52, 152, 219);        ///< The border color when the widget is focused.
    Color selectionColor_ = Color(52, 152, 219, 100); ///< The background color for selected text.
    Color cursorColor_ = colors::Black;             ///< The color of the cursor.
    
    render::FontStyle font_;        ///< The font style for the text.
    uint32_t padding_ = 8;          ///< The internal padding between the text and the border.
    float borderWidth_ = 1.0f;      ///< The width of the border.
    Color borderColor_ = Color(189, 195, 199); ///< The default border color.
    float borderRadius_ = 4.0f;     ///< The radius of the border corners.
    
    // Callbacks
    TextChangedCallback onTextChanged_; ///< Callback invoked on text change.
    EnterCallback onEnter_;             ///< Callback invoked when Enter is pressed.

public:
    /**
     * @brief Constructs a new TextInput object.
     */
    TextInput();
    ~TextInput() override;

    // Text content
    /**
     * @brief Sets the text content of the input field.
     * @param[in] text The new text to display.
     */
    void setText(std::wstring_view text);

    /**
     * @brief Gets the current text content.
     * @return A constant reference to the text string.
     */
    const std::wstring& getText() const noexcept { return text_; }

    /**
     * @brief Clears all text from the input field.
     */
    void clear() { setText(L""); }

    // Placeholder
    void setPlaceholder(const std::wstring& placeholder) { placeholder_ = placeholder; }
    const std::wstring& getPlaceholder() const noexcept { return placeholder_; }

    // Cursor
    /**
     * @brief Sets the position of the editing cursor.
     * @param[in] pos The new cursor position (will be clamped to valid range).
     */
    void setCursorPosition(size_t pos);
    size_t getCursorPosition() const noexcept { return cursorPos_; }

    // Selection
    /**
     * @brief Selects a range of text.
     * @param[in] start The starting index of the selection.
     * @param[in] end The ending index of the selection.
     */
    void setSelection(size_t start, size_t end);
    void clearSelection();
    bool hasSelection() const noexcept { return hasSelection_; }
    std::wstring getSelectedText() const;
    void deleteSelection();

    // Focus
    void setFocus(bool focus);
    bool isFocused() const noexcept { return focused_; }

    // Enabled state
    void setEnabled(bool enabled) noexcept { enabled_ = enabled; }
    bool isEnabled() const noexcept { return enabled_; }

    // Styling
    void setTextColor(const Color& color) noexcept { textColor_ = color; }
    void setPlaceholderColor(const Color& color) noexcept { placeholderColor_ = color; }
    void setFocusColor(const Color& color) noexcept { focusColor_ = color; }
    void setSelectionColor(const Color& color) noexcept { selectionColor_ = color; }
    void setBorderRadius(float radius) noexcept { borderRadius_ = radius; }
    void setPadding(uint32_t padding) noexcept { padding_ = padding; }
    
    void setFont(const render::FontStyle& font) noexcept { font_ = font; }
    void setFontSize(float size) noexcept { font_.size = size; }

    // Callbacks
    void setOnTextChanged(TextChangedCallback callback) { onTextChanged_ = std::move(callback); }
    void setOnEnter(EnterCallback callback) { onEnter_ = std::move(callback); }

    // Text operations
    void insertText(const std::wstring& text);
    void deleteChar(bool forward = false);
    void selectAll();
    void copy() const;
    void cut();
    void paste();

    // Event handling
    bool onEvent(const event::Event& event) override;

    // Rendering
    void render(Renderer& renderer) override;

private:
    /**
     * @brief Handles mouse button press and release events for focus and cursor placement.
     * @return True if the event was handled.
     */
    bool handleMouseEvent(const event::MouseButtonEvent& evt);

    /**
     * @brief Handles mouse move events for text selection.
     * @return True if the event was handled.
     */
    bool handleMouseMove(const event::MouseMoveEvent& evt);

    /**
     * @brief Handles key press events for text input and control actions.
     * @return True if the event was handled.
     */
    bool handleKeyEvent(const event::KeyEvent& evt);
    
    /**
     * @brief Moves the cursor left or right.
     * @param[in] delta The number of characters to move by (can be negative).
     * @param[in] extendSelection If true, the selection is extended/shrunk instead of cleared.
     */
    void moveCursor(int32_t delta, bool extendSelection = false);

    /**
     * @brief Manages the blinking state of the cursor.
     */
    void updateCursorBlink();

    /**
     * @brief Calculates the character index corresponding to a point in the widget.
     * @param[in] point The point from a mouse event.
     * @return The character index nearest to the point.
     */
    size_t getCursorPosFromPoint(const Point<int32_t>& point) const;
    
    /**
     * @brief Invokes the onTextChanged callback if it is set.
     */
    void notifyTextChanged();

    /**
     * @brief Ensures the cursor position is within the valid range [0, text.length()].
     */
    void clampCursor();
};

} // namespace frqs::widget