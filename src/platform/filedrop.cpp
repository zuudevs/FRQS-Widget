// src/platform/win32/file_drop.cpp
#pragma once

#include <windows.h>
#include <shellapi.h>
#include <filesystem>
#include <vector>
#include <string>

namespace frqs::platform {

// ============================================================================
// FILE DROP EVENT DATA
// ============================================================================

struct FileDropData {
    std::vector<std::filesystem::path> files;
    int32_t mouseX;
    int32_t mouseY;
};

// ============================================================================
// EXTRACT FILES FROM HDROP
// ============================================================================

inline FileDropData extractFileDropData(HWND hwnd, HDROP hDrop) {
    FileDropData data;
    
    // Get drop position
    POINT pt;
    if (DragQueryPoint(hDrop, &pt)) {
        data.mouseX = pt.x;
        data.mouseY = pt.y;
    }
    
    // Get number of files
    UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
    data.files.reserve(fileCount);
    
    // Extract each file path
    for (UINT i = 0; i < fileCount; ++i) {
        // Get required buffer size
        UINT pathLen = DragQueryFileW(hDrop, i, nullptr, 0);
        
        if (pathLen > 0) {
            // Allocate buffer and retrieve path
            std::wstring buffer(pathLen + 1, L'\0');
            DragQueryFileW(hDrop, i, buffer.data(), pathLen + 1);
            
            // Convert to filesystem::path (UTF-8 safe)
            try {
                data.files.emplace_back(buffer);
            } catch (const std::exception&) {
                // Skip malformed paths
            }
        }
    }
    
    // Cleanup
    DragFinish(hDrop);
    
    return data;
}

// ============================================================================
// WIN32 MESSAGE HANDLER (to be integrated in window_impl.cpp)
// ============================================================================

// Add this to your Win32 WndProc:
/*

case WM_DROPFILES: {
    HDROP hDrop = reinterpret_cast<HDROP>(wParam);
    
    auto dropData = platform::extractFileDropData(hwnd, hDrop);
    
    // Convert to event system
    event::FileDropEvent evt{
        std::move(dropData.files),
        widget::Point<int32_t>(dropData.mouseX, dropData.mouseY)
    };
    
    // Dispatch to widget tree
    if (pImpl->rootWidget) {
        pImpl->rootWidget->onEvent(event::Event(std::move(evt)));
    }
    
    return 0;
}

*/

// ============================================================================
// WIDGET-SIDE HANDLER (example usage)
// ============================================================================

// In your Widget class:
/*

class FileDropTarget : public Widget {
public:
    bool onEvent(const event::Event& event) override {
        if (auto* dropEvt = std::get_if<event::FileDropEvent>(&event)) {
            // Check if drop is inside this widget
            if (isPointInside(dropEvt->payload->dropPosition)) {
                handleFileDrop(dropEvt->payload->filePaths);
                return true;  // Event handled
            }
        }
        return Widget::onEvent(event);
    }
    
private:
    void handleFileDrop(const std::vector<std::filesystem::path>& files) {
        for (const auto& file : files) {
            // Process each file
            if (std::filesystem::is_regular_file(file)) {
                // Handle file
            } else if (std::filesystem::is_directory(file)) {
                // Handle directory
            }
        }
    }
};

*/

} // namespace frqs::platform