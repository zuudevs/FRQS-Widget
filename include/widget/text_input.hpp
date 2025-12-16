#pragma once

#include "iwidget.hpp"
#include "render/renderer.hpp"
#include <functional>

namespace frqs::widget {

// ============================================================================
// TEXT INPUT WIDGET (Single-line text input with cursor and selection)
// ============================================================================

class TextInput : public Widget {
public:
    using TextChangedCallback = std::function<void(const std::wstring&)>;
    using EnterCallback = std::function<void(const std::wstring&)>;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;

    // Text content
    std::wstring text_;
    std::wstring placeholder_ = L"Type here...";
    
    // Cursor and selection
    size_t cursorPos_ = 0;
    size_t selectionStart_ = 0;
    bool hasSelection_ = false;
    
    // State
    bool focused_ = false;
    bool enabled_ = true;
    bool cursorVisible_ = true;
    
    // Styling
    Color textColor_ = colors::Black;
    Color placeholderColor_ = Color(149, 165, 166);
    Color backgroundColor_ = colors::White;
    Color focusColor_ = Color(52, 152, 219);
    Color selectionColor_ = Color(52, 152, 219, 100);
    Color cursorColor_ = colors::Black;
    
    render::FontStyle font_;
    uint32_t padding_ = 8;
    float borderWidth_ = 1.0f;
    Color borderColor_ = Color(189, 195, 199);
    float borderRadius_ = 4.0f;
    
    // Callbacks
    TextChangedCallback onTextChanged_;
    EnterCallback onEnter_;

public:
    TextInput();
    ~TextInput() override;

    // Text content
    void setText(const std::wstring& text);
    const std::wstring& getText() const noexcept { return text_; }
    void clear() { setText(L""); }

    // Placeholder
    void setPlaceholder(const std::wstring& placeholder) { placeholder_ = placeholder; }
    const std::wstring& getPlaceholder() const noexcept { return placeholder_; }

    // Cursor
    void setCursorPosition(size_t pos);
    size_t getCursorPosition() const noexcept { return cursorPos_; }

    // Selection
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
    void deleteChar(bool forward = false);  // false = backspace, true = delete
    void selectAll();
    void copy() const;  // Copy to clipboard
    void cut();         // Cut to clipboard
    void paste();       // Paste from clipboard

    // Event handling
    bool onEvent(const event::Event& event) override;

    // Rendering
    void render(Renderer& renderer) override;

private:
    bool handleMouseEvent(const event::MouseButtonEvent& evt);
    bool handleMouseMove(const event::MouseMoveEvent& evt);
    bool handleKeyEvent(const event::KeyEvent& evt);
    
    void moveCursor(int32_t delta, bool extendSelection = false);
    void updateCursorBlink();
    size_t getCursorPosFromPoint(const Point<int32_t>& point) const;
    
    void notifyTextChanged();
    void clampCursor();
};

} // namespace frqs::widget