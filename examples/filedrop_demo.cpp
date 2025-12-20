// examples/filedrop_demo.cpp - FileDrop Demo
#include "frqs-widget.hpp"
#include "widget/container.hpp"
#include "widget/label.hpp"
#include "widget/button.hpp"
#include "widget/scroll_view.hpp"
#include <print>
#include <format>
#include <filesystem>

using namespace frqs;
using namespace frqs::widget;

// ============================================================================
// FILE DROP DEMO APPLICATION
// ============================================================================

class FileDropDemo {
private:
    std::shared_ptr<Label> dropZoneLabel_;
    std::shared_ptr<Container> fileListContainer_;
    std::shared_ptr<Label> statusLabel_;
    std::shared_ptr<Label> statsLabel_;
    
    std::vector<std::wstring> droppedFiles_;
    
public:
    void run() {
        try {
            std::println("=== FRQS-Widget: FileDrop Demo ===");
            std::println("");
            std::println("Features Demonstrated:");
            std::println("  • Drag & Drop file support");
            std::println("  • Multiple file handling");
            std::println("  • File info display");
            std::println("  • Drop zone visual feedback");
            std::println("");
            
            auto& app = Application::instance();
            app.initialize();
            
            // Create window
            WindowParams params;
            params.title = L"FileDrop Demo - Drag Files Here";
            params.size = Size(800u, 600u);
            params.position = Point(100, 100);
            
            auto window = app.createWindow(params);
            
            // Build UI
            buildUI(window);
            
            // Setup event handler
            setupEventHandler(window);
            
            window->show();
            
            std::println("✓ Demo window created");
            std::println("");
            std::println("Try:");
            std::println("  - Drag files from Explorer into the window");
            std::println("  - Drop multiple files at once");
            std::println("  - Watch file info appear in the list");
            std::println("");
            std::println("Close the window to exit.\n");
            
            app.run();
            
            std::println("\nDemo ended successfully.");
            
        } catch (const std::exception& e) {
            std::println(stderr, "ERROR: {}", e.what());
        }
    }

private:
    void buildUI(std::shared_ptr<core::Window> window) {
        // Root container
        auto root = createFlexColumn(0, 0);
        root->setRect(window->getClientRect());
        root->setBackgroundColor(Color(245, 245, 245));
        
        // ====================================================================
        // HEADER
        // ====================================================================
        auto header = std::make_shared<Label>(L"📁 File Drop Zone");
        header->setLayoutWeight(0.0f);
        header->setRect(Rect(0, 0, 800u, 60u));
        header->setBackgroundColor(Color(52, 152, 219));
        header->setTextColor(colors::White);
        header->setFontSize(24.0f);
        header->setFontBold(true);
        header->setAlignment(Label::Alignment::Center);
        header->setVerticalAlignment(Label::VerticalAlignment::Middle);
        root->addChild(header);
        
        // ====================================================================
        // DROP ZONE
        // ====================================================================
        auto dropZoneContainer = std::make_shared<Container>();
        dropZoneContainer->setLayoutWeight(0.0f);
        dropZoneContainer->setRect(Rect(0, 0, 800u, 200u));
        dropZoneContainer->setBackgroundColor(Color(236, 240, 241));
        dropZoneContainer->setBorder(Color(52, 152, 219), 3.0f);
        dropZoneContainer->setLayout(std::make_unique<FlexLayout>(
            FlexLayout::Direction::Column, 0, 0
        ));
        
        dropZoneLabel_ = std::make_shared<Label>(
            L"🎯 Drag & Drop Files Here\n\n"
            L"Drop single or multiple files\n"
            L"Supported: All file types"
        );
        dropZoneLabel_->setLayoutWeight(1.0f);
        dropZoneLabel_->setTextColor(Color(127, 140, 141));
        dropZoneLabel_->setFontSize(18.0f);
        dropZoneLabel_->setAlignment(Label::Alignment::Center);
        dropZoneLabel_->setVerticalAlignment(Label::VerticalAlignment::Middle);
        dropZoneContainer->addChild(dropZoneLabel_);
        
        root->addChild(dropZoneContainer);
        
        // ====================================================================
        // CONTROL PANEL
        // ====================================================================
        auto controlPanel = createFlexRow(10, 15);
        controlPanel->setLayoutWeight(0.0f);
        controlPanel->setBackgroundColor(Color(44, 62, 80));
        
        statsLabel_ = std::make_shared<Label>(L"Files dropped: 0");
        statsLabel_->setLayoutWeight(1.0f);
        statsLabel_->setTextColor(colors::White);
        statsLabel_->setFontSize(14.0f);
        statsLabel_->setVerticalAlignment(Label::VerticalAlignment::Middle);
        controlPanel->addChild(statsLabel_);
        
        auto btnClear = std::make_shared<Button>(L"Clear List");
        btnClear->setLayoutWeight(0.0f);
        btnClear->setRect(Rect(0, 0, 120u, 35u));
        btnClear->setNormalColor(Color(231, 76, 60));
        btnClear->setHoverColor(Color(192, 57, 43));
        btnClear->setOnClick([this]() {
            clearFiles();
        });
        controlPanel->addChild(btnClear);
        
        root->addChild(controlPanel);
        
        // ====================================================================
        // FILE LIST (ScrollView)
        // ====================================================================
        auto scrollView = std::make_shared<ScrollView>();
        scrollView->setLayoutWeight(1.0f);
        scrollView->setBackgroundColor(colors::White);
        
        fileListContainer_ = createFlexColumn(5, 10);
        fileListContainer_->setBackgroundColor(colors::White);
        
        // Empty state
        auto emptyLabel = std::make_shared<Label>(
            L"No files dropped yet.\nDrag files here to see them listed."
        );
        emptyLabel->setTextColor(Color(149, 165, 166));
        emptyLabel->setAlignment(Label::Alignment::Center);
        emptyLabel->setVerticalAlignment(Label::VerticalAlignment::Middle);
        fileListContainer_->addChild(emptyLabel);
        
        scrollView->setContent(fileListContainer_);
        root->addChild(scrollView);
        
        // ====================================================================
        // STATUS BAR
        // ====================================================================
        statusLabel_ = std::make_shared<Label>(
            L"Ready. Drag files from Explorer into the drop zone above."
        );
        statusLabel_->setLayoutWeight(0.0f);
        statusLabel_->setRect(Rect(0, 0, 800u, 40u));
        statusLabel_->setBackgroundColor(Color(52, 73, 94));
        statusLabel_->setTextColor(colors::White);
        statusLabel_->setPadding(10);
        statusLabel_->setVerticalAlignment(Label::VerticalAlignment::Middle);
        root->addChild(statusLabel_);
        
        // Set root
        window->setRootWidget(root);
    }
    
    void setupEventHandler(std::shared_ptr<core::Window> window) {
        // Get root widget and subscribe to FileDrop events
        auto root = window->getRootWidget();
        if (!root) return;
        
        // Note: This is a placeholder. The actual FileDrop event handling
        // will be implemented in win32_window.cpp with WM_DROPFILES message.
        // For now, we'll use a button to simulate file drops for testing.
        
        std::println("FileDrop event handler setup (waiting for implementation)");
    }
    
    void handleFileDrop(const std::vector<std::wstring>& files) {
        droppedFiles_.insert(droppedFiles_.end(), files.begin(), files.end());
        
        // Clear existing file list
        if (fileListContainer_) {
            // Remove all children
            auto children = fileListContainer_->getChildren();
            for (auto& child : children) {
                fileListContainer_->removeChild(child.get());
            }
            
            // Add new file items
            for (const auto& file : droppedFiles_) {
                addFileItem(file);
            }
            
            // Update content size
            uint32_t itemHeight = 60;
            uint32_t totalHeight = static_cast<uint32_t>(droppedFiles_.size()) * itemHeight 
                                 + static_cast<uint32_t>(droppedFiles_.size() - 1) * 5  // gaps
                                 + 20;  // padding
            
            fileListContainer_->setRect(Rect(0, 0, 760u, totalHeight));
        }
        
        // Update stats
        if (statsLabel_) {
            statsLabel_->setText(std::format(L"Files dropped: {}", droppedFiles_.size()));
        }
        
        // Update status
        if (statusLabel_) {
            statusLabel_->setText(std::format(
                L"✓ Received {} file(s). Total: {}",
                files.size(),
                droppedFiles_.size()
            ));
        }
        
        std::println("Files dropped: {}", files.size());
        for (const auto& file : files) {
            std::println("  - {}", std::string(file.begin(), file.end()));
        }
    }
    
    void addFileItem(const std::wstring& filePath) {
        auto fileItem = createFlexRow(10, 10);
        fileItem->setLayoutWeight(0.0f);
        fileItem->setRect(Rect(0, 0, 740u, 60u));
        fileItem->setBackgroundColor(Color(249, 249, 249));
        fileItem->setBorder(Color(220, 220, 220), 1.0f);
        
        // File icon (based on extension)
        std::wstring icon = getFileIcon(filePath);
        auto iconLabel = std::make_shared<Label>(icon);
        iconLabel->setLayoutWeight(0.0f);
        iconLabel->setRect(Rect(0, 0, 40u, 60u));
        iconLabel->setFontSize(24.0f);
        iconLabel->setAlignment(Label::Alignment::Center);
        iconLabel->setVerticalAlignment(Label::VerticalAlignment::Middle);
        fileItem->addChild(iconLabel);
        
        // File info
        auto infoContainer = createFlexColumn(2, 0);
        infoContainer->setLayoutWeight(1.0f);
        
        // Filename
        auto fileName = getFileName(filePath);
        auto nameLabel = std::make_shared<Label>(fileName);
        nameLabel->setLayoutWeight(0.0f);
        nameLabel->setFontSize(14.0f);
        nameLabel->setFontBold(true);
        nameLabel->setTextColor(Color(44, 62, 80));
        infoContainer->addChild(nameLabel);
        
        // Full path
        auto pathLabel = std::make_shared<Label>(filePath);
        pathLabel->setLayoutWeight(0.0f);
        pathLabel->setFontSize(11.0f);
        pathLabel->setTextColor(Color(127, 140, 141));
        infoContainer->addChild(pathLabel);
        
        // File size (if file exists)
        try {
            auto fileSize = std::filesystem::file_size(filePath);
            auto sizeText = formatFileSize(fileSize);
            auto sizeLabel = std::make_shared<Label>(sizeText);
            sizeLabel->setLayoutWeight(0.0f);
            sizeLabel->setFontSize(11.0f);
            sizeLabel->setTextColor(Color(52, 152, 219));
            infoContainer->addChild(sizeLabel);
        } catch (...) {
            // File might not exist or be inaccessible
        }
        
        fileItem->addChild(infoContainer);
        
        fileListContainer_->addChild(fileItem);
    }
    
    std::wstring getFileName(const std::wstring& path) {
        size_t lastSlash = path.find_last_of(L"\\/");
        if (lastSlash != std::wstring::npos) {
            return path.substr(lastSlash + 1);
        }
        return path;
    }
    
    std::wstring getFileIcon(const std::wstring& path) {
        // Get extension
        size_t lastDot = path.find_last_of(L'.');
        if (lastDot == std::wstring::npos) return L"📄";
        
        std::wstring ext = path.substr(lastDot);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        // Return icon based on extension
        if (ext == L".txt") return L"📝";
        if (ext == L".pdf") return L"📕";
        if (ext == L".doc" || ext == L".docx") return L"📘";
        if (ext == L".xls" || ext == L".xlsx") return L"📊";
        if (ext == L".ppt" || ext == L".pptx") return L"📙";
        if (ext == L".zip" || ext == L".rar" || ext == L".7z") return L"🗜️";
        if (ext == L".jpg" || ext == L".jpeg" || ext == L".png" || ext == L".gif") return L"🖼️";
        if (ext == L".mp3" || ext == L".wav" || ext == L".flac") return L"🎵";
        if (ext == L".mp4" || ext == L".avi" || ext == L".mkv") return L"🎬";
        if (ext == L".cpp" || ext == L".h" || ext == L".hpp") return L"👨‍💻";
        if (ext == L".py") return L"🐍";
        if (ext == L".js" || ext == L".ts") return L"📜";
        if (ext == L".html" || ext == L".css") return L"🌐";
        
        return L"📄";
    }
    
    std::wstring formatFileSize(uintmax_t bytes) {
        const wchar_t* units[] = {L"B", L"KB", L"MB", L"GB", L"TB"};
        int unitIndex = 0;
        double size = static_cast<double>(bytes);
        
        while (size >= 1024.0 && unitIndex < 4) {
            size /= 1024.0;
            unitIndex++;
        }
        
        if (unitIndex == 0) {
            return std::format(L"{} {}", static_cast<int>(size), units[unitIndex]);
        } else {
            return std::format(L"{:.2f} {}", size, units[unitIndex]);
        }
    }
    
    void clearFiles() {
        droppedFiles_.clear();
        
        if (fileListContainer_) {
            auto children = fileListContainer_->getChildren();
            for (auto& child : children) {
                fileListContainer_->removeChild(child.get());
            }
            
            // Add empty state
            auto emptyLabel = std::make_shared<Label>(
                L"No files dropped yet.\nDrag files here to see them listed."
            );
            emptyLabel->setTextColor(Color(149, 165, 166));
            emptyLabel->setAlignment(Label::Alignment::Center);
            emptyLabel->setVerticalAlignment(Label::VerticalAlignment::Middle);
            fileListContainer_->addChild(emptyLabel);
            
            fileListContainer_->setRect(Rect(0, 0, 760u, 200u));
        }
        
        if (statsLabel_) {
            statsLabel_->setText(L"Files dropped: 0");
        }
        
        if (statusLabel_) {
            statusLabel_->setText(L"File list cleared. Ready for new files.");
        }
        
        std::println("File list cleared");
    }
};

// ============================================================================
// MAIN
// ============================================================================

int main() {
    FileDropDemo demo;
    demo.run();
    return 0;
}