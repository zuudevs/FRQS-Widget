#pragma once
#include <filesystem>
#include <vector>
#include <expected>
#include <string>
#include <windows.h>
#include <shellapi.h>

namespace frqs::event {

// ============================================================================
// File Drop Event Definition
// ============================================================================
struct FileDropEvent {
    std::vector<std::filesystem::path> files;
    int mouse_x;
    int mouse_y;
    
    bool has_files() const noexcept { return !files.empty(); }
    size_t file_count() const noexcept { return files.size(); }
};

// ============================================================================
// Win32 File Drop Handler (platform/win32/win32_file_drop.cpp)
// ============================================================================
class Win32FileDropHandler {
public:
    // Call this in your WM_CREATE handler
    static void enable_drop(HWND hwnd) {
        ::DragAcceptFiles(hwnd, TRUE);
    }
    
    // Call this in your WndProc for WM_DROPFILES
    static std::expected<FileDropEvent, std::string> process_drop(WPARAM wparam, LPARAM lparam) {
        HDROP hdrop = reinterpret_cast<HDROP>(wparam);
        FileDropEvent evt;
        
        // Get drop position
        POINT pt;
        if (!::DragQueryPoint(hdrop, &pt)) {
            ::DragFinish(hdrop);
            return std::unexpected("Failed to query drop point");
        }
        
        evt.mouse_x = pt.x;
        evt.mouse_y = pt.y;
        
        // Get file count
        UINT file_count = ::DragQueryFileW(hdrop, 0xFFFFFFFF, nullptr, 0);
        if (file_count == 0) {
            ::DragFinish(hdrop);
            return std::unexpected("No files in drop");
        }
        
        evt.files.reserve(file_count);
        
        // Extract each file path
        for (UINT i = 0; i < file_count; ++i) {
            UINT length = ::DragQueryFileW(hdrop, i, nullptr, 0);
            if (length == 0) continue;
            
            std::wstring buffer(length + 1, L'\0');
            UINT copied = ::DragQueryFileW(hdrop, i, buffer.data(), length + 1);
            
            if (copied > 0) {
                buffer.resize(copied);
                evt.files.emplace_back(std::move(buffer));
            }
        }
        
        ::DragFinish(hdrop);
        return evt;
    }
};

} // namespace frqs::event

// ============================================================================
// Integration in win32_window.cpp WndProc
// ============================================================================
/*
LRESULT CALLBACK Win32Window::window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    auto* window = reinterpret_cast<Win32Window*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    
    switch (msg) {
        case WM_CREATE: {
            // Enable drag-drop
            Win32FileDropHandler::enable_drop(hwnd);
            return 0;
        }
        
        case WM_DROPFILES: {
            if (!window) return 0;
            
            auto result = Win32FileDropHandler::process_drop(wp, lp);
            if (result.has_value()) {
                // Dispatch to event system
                window->dispatch_file_drop(std::move(result.value()));
            } else {
                // Log error
                window->log_error(result.error());
            }
            return 0;
        }
        
        // ... other messages
    }
    
    return ::DefWindowProcW(hwnd, msg, wp, lp);
}

void Win32Window::dispatch_file_drop(FileDropEvent&& evt) {
    // Find widget at drop position
    if (auto* target = hit_test(evt.mouse_x, evt.mouse_y)) {
        target->on_file_drop(evt);
    }
    
    // Also send to event bus for global handlers
    event_bus_.publish(std::move(evt));
}
*/

// ============================================================================
// Widget Base Class Integration
// ============================================================================
/*
namespace frqs::ui {

class Widget {
public:
    virtual ~Widget() = default;
    
    // Override in derived classes to handle file drops
    virtual void on_file_drop(const event::FileDropEvent& evt) {
        // Default: do nothing
    }
    
    // ... other methods
};

// Example: File Drop Zone Widget
class FileDropZone : public Widget {
public:
    void on_file_drop(const event::FileDropEvent& evt) override {
        for (const auto& path : evt.files) {
            // Process dropped file
            if (std::filesystem::is_regular_file(path)) {
                handle_file(path);
            }
        }
    }
    
private:
    void handle_file(const std::filesystem::path& path) {
        // Your file processing logic
    }
};

} // namespace frqs::ui
*/