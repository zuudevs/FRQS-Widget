// src/widget/text_input.cpp - COMPLETE WITH DIRECTWRITE MEASUREMENT
#include "widget/text_input.hpp"
#include "event/event_types.hpp"
#include "render/resource_cache.hpp"
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
    
    // ✅ Cache renderer pointer for text measurement
    render::IExtendedRenderer* extRenderer = nullptr;
    
    Impl() : lastBlinkTime(std::chrono::steady_clock::now()) {}
};

// ============================================================================
// CONSTRUCTOR / DESTRUCTOR
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

// ============================================================================
// TEXT MANAGEMENT
// ============================================================================

void TextInput::setText(std::wstring_view text) {
    if (text_ == text) return;
    
    text_ = text;
    clearSelection();
    cursorPos_ = text_.length();
    clampCursor();
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
    
    try {
        size_t start = std::min(selectionStart_, cursorPos_);
        size_t end = std::max(selectionStart_, cursorPos_);
        
        if (start >= text_.length()) {
            clearSelection();
            return;
        }
        
        end = std::min(end, text_.length());
        
        if (start < end && start < text_.length()) {
            text_.erase(start, end - start);
            cursorPos_ = start;
            hasSelection_ = false;
            notifyTextChanged();
            invalidate();
        } else {
            hasSelection_ = false;
        }
    } catch (...) {
        hasSelection_ = false;
        clampCursor();
    }
}

// ============================================================================
// FOCUS MANAGEMENT
// ============================================================================

void TextInput::setFocus(bool focus) {
    if (focused_ == focus) return;
    
    focused_ = focus;
    
    if (!focused_) {
        clearSelection();
    }
    
    updateCursorBlink();
    invalidate();
}

// ============================================================================
// TEXT OPERATIONS
// ============================================================================

void TextInput::insertText(const std::wstring& text) {
    if (!enabled_ || text.empty()) return;
    
    try {
        if (hasSelection_) {
            deleteSelection();
        }
        
        if (cursorPos_ > text_.length()) {
            cursorPos_ = text_.length();
        }
        
        if (cursorPos_ == text_.length()) {
            text_ += text;
            cursorPos_ = text_.length();
        } else {
            text_.insert(cursorPos_, text);
            cursorPos_ += text.length();
        }
        
        notifyTextChanged();
        invalidate();
        
    } catch (...) {
        clampCursor();
    }
}

void TextInput::deleteChar(bool forward) {
    if (!enabled_ || text_.empty()) return;
    
    if (hasSelection_) {
        deleteSelection();
        return;
    }
    
    try {
        if (forward) {
            if (cursorPos_ < text_.length()) {
                text_.erase(cursorPos_, 1);
                clampCursor();
                notifyTextChanged();
                invalidate();
            }
        } else {
            if (cursorPos_ > 0 && cursorPos_ <= text_.length()) {
                text_.erase(cursorPos_ - 1, 1);
                --cursorPos_;
                notifyTextChanged();
                invalidate();
            }
        }
    } catch (...) {
        clampCursor();
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
    // TODO: Implement clipboard
}

void TextInput::cut() {
    copy();
    deleteSelection();
}

void TextInput::paste() {
    // TODO: Implement clipboard
}

// ============================================================================
// CURSOR MOVEMENT
// ============================================================================

void TextInput::moveCursor(int32_t delta, bool extendSelection) {
    if (delta == 0) return;
    
    if (!extendSelection && hasSelection_) {
        if (delta < 0) {
            cursorPos_ = std::min(selectionStart_, cursorPos_);
        } else {
            cursorPos_ = std::max(selectionStart_, cursorPos_);
        }
        clearSelection();
    } else {
        size_t oldPos = cursorPos_;
        
        if (delta < 0) {
            if (cursorPos_ > 0) {
                cursorPos_ = static_cast<size_t>(
                    std::max<int64_t>(0, static_cast<int64_t>(cursorPos_) + delta)
                );
            }
        } else {
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
        try {
            onTextChanged_(text_);
        } catch (...) {
            // Swallow exceptions
        }
    }
}

void TextInput::clampCursor() {
    cursorPos_ = std::min(cursorPos_, text_.length());
}

// ============================================================================
// ✅ PRECISE CURSOR POSITION (DirectWrite Hit-Testing)
// ============================================================================

size_t TextInput::getCursorPosFromPoint(const Point<int32_t>& point) const {
    auto rect = getRect();
    int32_t relX = point.x - rect.x - static_cast<int32_t>(padding_);
    
    if (relX <= 0) return 0;
    
    // ✅ Use DirectWrite for EXACT hit testing
    if (pImpl_->extRenderer) {
        size_t pos = pImpl_->extRenderer->getCharPositionFromX(
            text_, 
            static_cast<float>(relX),
            font_
        );
        return std::min(pos, text_.length());
    }
    
    // ❌ Fallback (should never execute)
    size_t pos = static_cast<size_t>(relX / 8);
    return std::min(pos, text_.length());
}

// ============================================================================
// EVENT HANDLING
// ============================================================================

bool TextInput::handleMouseEvent(const event::MouseButtonEvent& evt) {
    auto rect = getRect();
    bool inside = evt.position.x >= rect.x && evt.position.x < static_cast<int32_t>(rect.getRight()) &&
                  evt.position.y >= rect.y && evt.position.y < static_cast<int32_t>(rect.getBottom());
    
    if (evt.action == event::MouseButtonEvent::Action::Press) {
        if (inside) {
            setFocus(true);
            
            size_t clickPos = getCursorPosFromPoint(evt.position);
            bool shift = (evt.modifiers & static_cast<uint32_t>(event::ModifierKey::Shift)) != 0;
            
            if (shift && hasSelection_) {
                cursorPos_ = clickPos;
            } else {
                cursorPos_ = clickPos;
                selectionStart_ = clickPos;
                clearSelection();
            }
            
            pImpl_->mouseDown = true;
            invalidate();
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
        size_t newPos = getCursorPosFromPoint(evt.position);
        if (newPos != cursorPos_) {
            cursorPos_ = newPos;
            if (cursorPos_ != selectionStart_) {
                hasSelection_ = true;
            } else {
                hasSelection_ = false;
            }
            invalidate();
        }
        return true;
    }
    
    return false;
}

// ============================================================================
// ✅ KEYBOARD EVENT HANDLER (WM_CHAR + Navigation)
// ============================================================================

bool TextInput::handleKeyEvent(const event::KeyEvent& evt) {
    if (!focused_ || !enabled_) return false;
    if (evt.action != event::KeyEvent::Action::Press && 
        evt.action != event::KeyEvent::Action::Repeat) {
        return false;
    }
    
    bool ctrl = (evt.modifiers & static_cast<uint32_t>(event::ModifierKey::Control)) != 0;
    bool shift = (evt.modifiers & static_cast<uint32_t>(event::ModifierKey::Shift)) != 0;
    bool isCharEvent = (evt.modifiers & 0x80000000) != 0;
    
    // ✅ CRITICAL: Handle WM_CHAR messages (actual character input)
    if (isCharEvent) {
        wchar_t ch = static_cast<wchar_t>(evt.keyCode & 0x7FFFFFFF);
        
        if (ch == 0) return false;
        
        // Enter key
        if (ch == L'\r' || ch == L'\n') {
            if (onEnter_) {
                onEnter_(text_);
            }
            return true;
        }
        
        // Backspace
        if (ch == L'\b') {
            deleteChar(false);
            return true;
        }
        
        // Tab - don't handle (let focus system handle)
        if (ch == L'\t') {
            return false;
        }
        
        // ✅ Printable characters (excluding control chars)
        if (ch >= 32 && ch != 0x7F) {
            insertText(std::wstring(1, ch));
            return true;
        }
        
        return false;
    }
    
    // ✅ Handle navigation keys (from WM_KEYDOWN)
    using KC = event::KeyCode;
    
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
    try {
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
    } catch (...) {
        return false;
    }
}

// ============================================================================
// ✅ RENDERING (Pixel-Perfect Cursor with DirectWrite)
// ============================================================================

void TextInput::render(Renderer& renderer) {
    if (!isVisible()) return;

    try {
        // ✅ Cache extended renderer for measurement
        if (!pImpl_->extRenderer) {
            pImpl_->extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer);
        }
        
        auto rect = getRect();
        
        // Background
        renderer.fillRect(rect, backgroundColor_);
        
        // Border
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
        
        // ✅ Render selection background
        if (hasSelection_ && focused_) {
            size_t start = std::min(selectionStart_, cursorPos_);
            size_t end = std::max(selectionStart_, cursorPos_);
            
            float selStartX = 0.0f;
            float selEndX = 0.0f;
            
            if (pImpl_->extRenderer) {
                // Use DirectWrite for exact measurement
                selStartX = pImpl_->extRenderer->measureTextWidth(text_, start, font_);
                selEndX = pImpl_->extRenderer->measureTextWidth(text_, end, font_);
            } else {
                // Fallback
                selStartX = static_cast<float>(start * 8);
                selEndX = static_cast<float>(end * 8);
            }
            
            int32_t selX = textRect.x + static_cast<int32_t>(selStartX);
            uint32_t selW = static_cast<uint32_t>(selEndX - selStartX);
            
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
        
        // ✅ RENDER CURSOR (Pixel-Perfect with DirectWrite)
        if (focused_ && cursorVisible_) {
            float cursorXOffset = 0.0f;
            
            if (pImpl_->extRenderer && cursorPos_ > 0) {
                // Use DirectWrite to measure EXACT width
                cursorXOffset = pImpl_->extRenderer->measureTextWidth(
                    text_, 
                    cursorPos_, 
                    font_
                );
            }
            
            int32_t cursorX = textRect.x + static_cast<int32_t>(cursorXOffset);
            
            Rect<int32_t, uint32_t> cursorRect(
                cursorX, 
                textRect.y + 2,
                2,
                textRect.h > 4 ? textRect.h - 4 : textRect.h
            );
            
            renderer.fillRect(cursorRect, cursorColor_);
        }
        
        // Update cursor blink
        if (focused_) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - pImpl_->lastBlinkTime
            );
            
            if (elapsed.count() > 500) {
                cursorVisible_ = !cursorVisible_;
                pImpl_->lastBlinkTime = now;
            }
        }
    } catch (...) {
        // Swallow render exceptions
    }
}

} // namespace frqs::widget