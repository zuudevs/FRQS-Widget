#include "widget/text_input.hpp"
#include "event/event_types.hpp"
#include <algorithm>
#include <chrono>

namespace frqs::widget {

// ============================================================================
// TEXT INPUT PIMPL
// ============================================================================

struct TextInput::Impl {
    std::chrono::steady_clock::time_point lastBlinkTime;
    Point<int32_t> lastMousePos;
    bool mouseDown = false;
    
    Impl() : lastBlinkTime(std::chrono::steady_clock::now()) {}
};

// ============================================================================
// TEXT INPUT IMPLEMENTATION
// ============================================================================

TextInput::TextInput()
    : Widget()
    , pImpl_(std::make_unique<Impl>())
{
    font_.size = 14.0f;
    font_.family = L"Segoe UI";
    setBackgroundColor(backgroundColor_);
}

TextInput::~TextInput() = default;

void TextInput::setText(const std::wstring& text) {
    if (text_ == text) return;
    
    text_ = text;
    clearSelection();
    setCursorPosition(text_.length());
    notifyTextChanged();
    invalidate();
}

void TextInput::setCursorPosition(size_t pos) {
    cursorPos_ = std::min(pos, text_.length());
    clearSelection();
    updateCursorBlink();
    invalidate();
}

void TextInput::setSelection(size_t start, size_t end) {
    start = std::min(start, text_.length());
    end = std::min(end, text_.length());
    
    if (start == end) {
        clearSelection();
        return;
    }
    
    selectionStart_ = std::min(start, end);
    cursorPos_ = std::max(start, end);
    hasSelection_ = true;
    invalidate();
}

void TextInput::clearSelection() {
    if (!hasSelection_) return;
    hasSelection_ = false;
    invalidate();
}

std::wstring TextInput::getSelectedText() const {
    if (!hasSelection_) return L"";
    
    size_t start = std::min(selectionStart_, cursorPos_);
    size_t end = std::max(selectionStart_, cursorPos_);
    
    if (start >= text_.length()) return L"";
    
    size_t len = std::min(end - start, text_.length() - start);
    return text_.substr(start, len);
}

void TextInput::deleteSelection() {
    if (!hasSelection_) return;
    
    size_t start = std::min(selectionStart_, cursorPos_);
    size_t end = std::max(selectionStart_, cursorPos_);
    
    text_.erase(start, end - start);
    cursorPos_ = start;
    clearSelection();
    notifyTextChanged();
    invalidate();
}

void TextInput::setFocus(bool focus) {
    if (focused_ == focus) return;
    focused_ = focus;
    
    if (!focused_) {
        clearSelection();
    }
    
    updateCursorBlink();
    invalidate();
}

void TextInput::insertText(const std::wstring& text) {
    if (!enabled_ || text.empty()) return;
    
    // Delete selection if present
    if (hasSelection_) {
        deleteSelection();
    }
    
    // Insert text at cursor
    text_.insert(cursorPos_, text);
    cursorPos_ += text.length();
    
    notifyTextChanged();
    invalidate();
}

void TextInput::deleteChar(bool forward) {
    if (!enabled_ || text_.empty()) return;
    
    if (hasSelection_) {
        deleteSelection();
        return;
    }
    
    if (forward) {
        // Delete key
        if (cursorPos_ < text_.length()) {
            text_.erase(cursorPos_, 1);
            notifyTextChanged();
            invalidate();
        }
    } else {
        // Backspace
        if (cursorPos_ > 0) {
            text_.erase(cursorPos_ - 1, 1);
            --cursorPos_;
            notifyTextChanged();
            invalidate();
        }
    }
}

void TextInput::selectAll() {
    if (text_.empty()) return;
    
    selectionStart_ = 0;
    cursorPos_ = text_.length();
    hasSelection_ = true;
    invalidate();
}

void TextInput::copy() const {
    // TODO: Implement clipboard copy
    // This requires Windows clipboard API
    (void)getSelectedText();
}

void TextInput::cut() {
    copy();
    deleteSelection();
}

void TextInput::paste() {
    // TODO: Implement clipboard paste
    // This requires Windows clipboard API
}

void TextInput::moveCursor(int32_t delta, bool extendSelection) {
    if (delta == 0) return;
    
    if (!extendSelection && hasSelection_ && !extendSelection) {
        // Move to start/end of selection
        if (delta < 0) {
            cursorPos_ = std::min(selectionStart_, cursorPos_);
        } else {
            cursorPos_ = std::max(selectionStart_, cursorPos_);
        }
        clearSelection();
    } else {
        size_t oldPos = cursorPos_;
        
        if (delta < 0) {
            // Move left
            if (cursorPos_ > 0) {
                cursorPos_ = static_cast<size_t>(
                    std::max<int64_t>(0, static_cast<int64_t>(cursorPos_) + delta)
                );
            }
        } else {
            // Move right
            cursorPos_ = std::min(cursorPos_ + static_cast<size_t>(delta), text_.length());
        }
        
        if (extendSelection) {
            if (!hasSelection_) {
                selectionStart_ = oldPos;
                hasSelection_ = true;
            }
        } else {
            clearSelection();
        }
    }
    
    updateCursorBlink();
    invalidate();
}

void TextInput::updateCursorBlink() {
    pImpl_->lastBlinkTime = std::chrono::steady_clock::now();
    cursorVisible_ = true;
}

void TextInput::notifyTextChanged() {
    if (onTextChanged_) {
        onTextChanged_(text_);
    }
}

void TextInput::clampCursor() {
    cursorPos_ = std::min(cursorPos_, text_.length());
}

size_t TextInput::getCursorPosFromPoint(const Point<int32_t>& point) const {
    // Simplified: return closest position
    // TODO: Implement proper text measurement
    auto rect = getRect();
    int32_t relX = point.x - rect.x - static_cast<int32_t>(padding_);
    
    if (relX <= 0) return 0;
    
    // Rough approximation: each char is ~8 pixels
    size_t pos = static_cast<size_t>(relX / 8);
    return std::min(pos, text_.length());
}

bool TextInput::handleMouseEvent(const event::MouseButtonEvent& evt) {
    auto rect = getRect();
    bool inside = evt.position.x >= rect.x && evt.position.x < rect.getRight() &&
                  evt.position.y >= rect.y && evt.position.y < rect.getBottom();
    
    if (evt.action == event::MouseButtonEvent::Action::Press) {
        if (inside) {
            setFocus(true);
            
            // Set cursor position from click
            size_t clickPos = getCursorPosFromPoint(evt.position);
            
            // Check for shift+click (extend selection)
            bool shift = (evt.modifiers & static_cast<uint32_t>(event::ModifierKey::Shift)) != 0;
            
            if (shift && hasSelection_) {
                cursorPos_ = clickPos;
            } else {
                setCursorPosition(clickPos);
                selectionStart_ = clickPos;
            }
            
            pImpl_->mouseDown = true;
            return true;
        } else {
            setFocus(false);
        }
    } else if (evt.action == event::MouseButtonEvent::Action::Release) {
        pImpl_->mouseDown = false;
        return focused_;
    }
    
    return false;
}

bool TextInput::handleMouseMove(const event::MouseMoveEvent& evt) {
    pImpl_->lastMousePos = evt.position;
    
    if (pImpl_->mouseDown && focused_) {
        // Extend selection while dragging
        size_t newPos = getCursorPosFromPoint(evt.position);
        if (newPos != cursorPos_) {
            cursorPos_ = newPos;
            if (cursorPos_ != selectionStart_) {
                hasSelection_ = true;
            } else {
                clearSelection();
            }
            invalidate();
        }
        return true;
    }
    
    return false;
}

// src/widget/text_input.cpp - FIXED handleKeyEvent method

bool TextInput::handleKeyEvent(const event::KeyEvent& evt) {
    if (!focused_ || !enabled_) return false;
    if (evt.action != event::KeyEvent::Action::Press && 
        evt.action != event::KeyEvent::Action::Repeat) {
        return false;
    }
    
    bool ctrl = (evt.modifiers & static_cast<uint32_t>(event::ModifierKey::Control)) != 0;
    bool shift = (evt.modifiers & static_cast<uint32_t>(event::ModifierKey::Shift)) != 0;
    bool isCharEvent = (evt.modifiers & 0x80000000) != 0; // Check if this is WM_CHAR
    
    using KC = event::KeyCode;
    
    // ✅ FIX: Handle WM_CHAR messages (actual character input)
    if (isCharEvent) {
        wchar_t ch = static_cast<wchar_t>(evt.keyCode & 0x7FFFFFFF);
        
        // Handle special characters
        if (ch == L'\r') {
            // Enter key
            if (onEnter_) {
                onEnter_(text_);
            }
            return true;
        } else if (ch == L'\b') {
            // Backspace
            deleteChar(false);
            return true;
        } else if (ch == L'\t') {
            // Tab - could be used for focus navigation
            return false; // Let parent handle it
        } else if (ch >= 32) {
            // Printable character
            insertText(std::wstring(1, ch));
            return true;
        }
        
        return false;
    }
    
    // ✅ Handle navigation keys (WM_KEYDOWN)
    switch (static_cast<KC>(evt.keyCode)) {
        case KC::Left:
            moveCursor(-1, shift);
            return true;
            
        case KC::Right:
            moveCursor(1, shift);
            return true;
            
        case KC::Home:
            if (shift) {
                selectionStart_ = cursorPos_;
                cursorPos_ = 0;
                hasSelection_ = (selectionStart_ != cursorPos_);
            } else {
                cursorPos_ = 0;
                clearSelection();
            }
            invalidate();
            return true;
            
        case KC::End:
            if (shift) {
                selectionStart_ = cursorPos_;
                cursorPos_ = text_.length();
                hasSelection_ = (selectionStart_ != cursorPos_);
            } else {
                cursorPos_ = text_.length();
                clearSelection();
            }
            invalidate();
            return true;
            
        case KC::Delete:
            deleteChar(true);
            return true;
            
        case KC::A:
            if (ctrl) {
                selectAll();
                return true;
            }
            break;
            
        case KC::C:
            if (ctrl && hasSelection_) {
                copy();
                return true;
            }
            break;
            
        case KC::X:
            if (ctrl && hasSelection_) {
                cut();
                return true;
            }
            break;
            
        case KC::V:
            if (ctrl) {
                paste();
                return true;
            }
            break;
            
        default:
            break;
    }
    
    return false;
}

bool TextInput::onEvent(const event::Event& event) {
    if (auto* mouseBtn = std::get_if<event::MouseButtonEvent>(&event)) {
        return handleMouseEvent(*mouseBtn);
    }
    
    if (auto* mouseMove = std::get_if<event::MouseMoveEvent>(&event)) {
        return handleMouseMove(*mouseMove);
    }
    
    if (auto* keyEvt = std::get_if<event::KeyEvent>(&event)) {
        return handleKeyEvent(*keyEvt);
    }
    
    return Widget::onEvent(event);
}

void TextInput::render(Renderer& renderer) {
    if (!isVisible()) return;

    auto rect = getRect();
    
    // Background
    renderer.fillRect(rect, backgroundColor_);
    
    // Border (changes color when focused)
    Color currentBorderColor = focused_ ? focusColor_ : borderColor_;
    
    if (auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer)) {
        extRenderer->drawRoundedRect(rect, borderRadius_, borderRadius_, 
                                    currentBorderColor, borderWidth_);
    } else {
        renderer.drawRect(rect, currentBorderColor, borderWidth_);
    }
    
    // Text area with padding
    auto textRect = Rect<int32_t, uint32_t>(
        rect.x + static_cast<int32_t>(padding_),
        rect.y + static_cast<int32_t>(padding_),
        rect.w > padding_ * 2 ? rect.w - padding_ * 2 : 0,
        rect.h > padding_ * 2 ? rect.h - padding_ * 2 : 0
    );
    
    if (textRect.w == 0 || textRect.h == 0) return;
    
    // Render selection
    if (hasSelection_ && focused_) {
        size_t start = std::min(selectionStart_, cursorPos_);
        size_t end = std::max(selectionStart_, cursorPos_);
        
        // Simplified selection rendering
        // TODO: Calculate exact text positions
        int32_t selX = textRect.x + static_cast<int32_t>(start * 8);
        uint32_t selW = static_cast<uint32_t>((end - start) * 8);
        
        Rect<int32_t, uint32_t> selRect(selX, textRect.y, selW, textRect.h);
        renderer.fillRect(selRect, selectionColor_);
    }
    
    // Render text or placeholder
    if (text_.empty() && !focused_) {
        if (auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer)) {
            extRenderer->drawTextEx(
                placeholder_, 
                textRect, 
                placeholderColor_,
                font_,
                render::TextAlign::Left,
                render::VerticalAlign::Middle
            );
        } else {
            renderer.drawText(placeholder_, textRect, placeholderColor_);
        }
    } else if (!text_.empty()) {
        if (auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer)) {
            extRenderer->drawTextEx(
                text_, 
                textRect, 
                textColor_,
                font_,
                render::TextAlign::Left,
                render::VerticalAlign::Middle
            );
        } else {
            renderer.drawText(text_, textRect, textColor_);
        }
    }
    
    // Render cursor
    if (focused_ && cursorVisible_) {
        // TODO: Calculate exact cursor position from text measurement
        int32_t cursorX = textRect.x + static_cast<int32_t>(cursorPos_ * 8);
        
        Rect<int32_t, uint32_t> cursorRect(
            cursorX, 
            textRect.y + 2,
            2,
            textRect.h > 4 ? textRect.h - 4 : textRect.h
        );
        
        renderer.fillRect(cursorRect, cursorColor_);
    }
    
    // Update cursor blink
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - pImpl_->lastBlinkTime
    );
    
    if (elapsed.count() > 500) {  // Blink every 500ms
        cursorVisible_ = !cursorVisible_;
        pImpl_->lastBlinkTime = now;
        invalidate();
    }
}

} // namespace frqs::widget