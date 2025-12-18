// examples/phase3_demo.cpp - Phase 3 Feature Demonstration
// Showcases: CheckBox, Image (WIC), and ScrollView with draggable scrollbars

#include "frqs-widget.hpp"
#include "widget/container.hpp"
#include "widget/label.hpp"
#include "widget/button.hpp"
#include "widget/checkbox.hpp"
#include "widget/image.hpp"
#include "widget/scroll_view.hpp"
#include <print>
#include <format>

using namespace frqs;
using namespace frqs::widget;

// ============================================================================
// PHASE 3 DEMO APPLICATION
// ============================================================================

class Phase3Demo {
private:
    std::shared_ptr<Label> statusLabel_;
    std::vector<std::shared_ptr<CheckBox>> checkboxes_;
    std::shared_ptr<Image> image_;

public:
    void run() {
        try {
            std::println("=== FRQS-Widget Phase 3 Demo ===");
            std::println("Features:");
            std::println("  • CheckBox widgets with callbacks");
            std::println("  • Image display with WIC (multiple scale modes)");
            std::println("  • ScrollView with draggable scrollbars");
            std::println("");
            
            auto& app = Application::instance();
            app.initialize();
            
            // Create main window
            WindowParams params;
            params.title = L"Phase 3 Demo - CheckBox, Image, ScrollView";
            params.size = Size(900u, 700u);
            params.position = Point(100, 100);
            
            auto window = app.createWindow(params);
            
            // Build UI
            buildUI(window);
            
            window->show();
            
            std::println("✓ Demo window created");
            std::println("Try the interactive features!");
            std::println("Close the window to exit.\n");
            
            app.run();
            
            std::println("\nDemo ended successfully.");
            
        } catch (const std::exception& e) {
            std::println(stderr, "ERROR: {}", e.what());
        }
    }

private:
    void buildUI(std::shared_ptr<core::Window> window) {
        // Root: Horizontal layout (sidebar + content)
        auto root = createFlexRow(0, 0);
        root->setRect(window->getClientRect());
        root->setBackgroundColor(Color(245, 245, 245));
        
        // ====================================================================
        // LEFT SIDEBAR (Fixed 300px)
        // ====================================================================
        auto sidebar = createFlexColumn(10, 15);
        sidebar->setLayoutWeight(0.0f);
        sidebar->setRect(Rect(0, 0, 300u, 700u));
        sidebar->setBackgroundColor(Color(52, 73, 94));
        
        // Sidebar title
        auto sidebarTitle = std::make_shared<Label>(L"Settings");
        sidebarTitle->setLayoutWeight(0.0f);
        sidebarTitle->setRect(Rect(0, 0, 270u, 40u));
        sidebarTitle->setTextColor(colors::White);
        sidebarTitle->setFontSize(18.0f);
        sidebarTitle->setFontBold(true);
        sidebar->addChild(sidebarTitle);
        
        // CheckBox group
        auto checkboxSection = createFlexColumn(8, 0);
        checkboxSection->setLayoutWeight(0.0f);
        checkboxSection->setBackgroundColor(Color(44, 62, 80));
        checkboxSection->setPadding(15);
        checkboxSection->setBorder(Color(34, 52, 70), 1.0f);
        
        auto checkboxLabel = std::make_shared<Label>(L"Options:");
        checkboxLabel->setTextColor(Color(149, 165, 166));
        checkboxLabel->setFontSize(12.0f);
        checkboxSection->addChild(checkboxLabel);
        
        // Create checkboxes
        auto cb1 = std::make_shared<CheckBox>(L"Enable notifications");
        cb1->setTextColor(colors::White);
        cb1->setChecked(true);
        cb1->setOnChanged([this](bool checked) {
            std::println("Notifications: {}", checked ? "ON" : "OFF");
            updateStatus(checked ? L"Notifications enabled" : L"Notifications disabled");
        });
        cb1->setRect(Rect(0, 0, 270u, 30u));
        checkboxes_.push_back(cb1);
        checkboxSection->addChild(cb1);
        
        auto cb2 = std::make_shared<CheckBox>(L"Auto-save changes");
        cb2->setTextColor(colors::White);
        cb2->setOnChanged([this](bool checked) {
            std::println("Auto-save: {}", checked ? "ON" : "OFF");
            updateStatus(checked ? L"Auto-save enabled" : L"Auto-save disabled");
        });
        cb2->setRect(Rect(0, 0, 270u, 30u));
        checkboxes_.push_back(cb2);
        checkboxSection->addChild(cb2);
        
        auto cb3 = std::make_shared<CheckBox>(L"Dark mode");
        cb3->setTextColor(colors::White);
        cb3->setChecked(true);
        cb3->setOnChanged([this](bool checked) {
            std::println("Dark mode: {}", checked ? "ON" : "OFF");
            updateStatus(checked ? L"Dark mode enabled" : L"Dark mode disabled");
        });
        cb3->setRect(Rect(0, 0, 270u, 30u));
        checkboxes_.push_back(cb3);
        checkboxSection->addChild(cb3);
        
        sidebar->addChild(checkboxSection);
        
        // Image scale mode buttons
        auto imageModeSection = createFlexColumn(8, 0);
        imageModeSection->setLayoutWeight(0.0f);
        imageModeSection->setBackgroundColor(Color(44, 62, 80));
        imageModeSection->setPadding(15);
        imageModeSection->setBorder(Color(34, 52, 70), 1.0f);
        
        auto imageModeLabel = std::make_shared<Label>(L"Image Scale Mode:");
        imageModeLabel->setTextColor(Color(149, 165, 166));
        imageModeLabel->setFontSize(12.0f);
        imageModeSection->addChild(imageModeLabel);
        
        // Fit button
        auto fitBtn = std::make_shared<Button>(L"Fit (Letterbox)");
        fitBtn->setNormalColor(Color(46, 204, 113));
        fitBtn->setHoverColor(Color(39, 174, 96));
        fitBtn->setRect(Rect(0, 0, 270u, 35u));
        fitBtn->setOnClick([this]() {
            if (image_) {
                image_->setScaleMode(Image::ScaleMode::Fit);
                updateStatus(L"Image mode: Fit (letterbox)");
                std::println("Image scale mode: Fit");
            }
        });
        imageModeSection->addChild(fitBtn);
        
        // Fill button
        auto fillBtn = std::make_shared<Button>(L"Fill (Crop)");
        fillBtn->setNormalColor(Color(52, 152, 219));
        fillBtn->setHoverColor(Color(41, 128, 185));
        fillBtn->setRect(Rect(0, 0, 270u, 35u));
        fillBtn->setOnClick([this]() {
            if (image_) {
                image_->setScaleMode(Image::ScaleMode::Fill);
                updateStatus(L"Image mode: Fill (crop)");
                std::println("Image scale mode: Fill");
            }
        });
        imageModeSection->addChild(fillBtn);
        
        // Stretch button
        auto stretchBtn = std::make_shared<Button>(L"Stretch");
        stretchBtn->setNormalColor(Color(231, 76, 60));
        stretchBtn->setHoverColor(Color(192, 57, 43));
        stretchBtn->setRect(Rect(0, 0, 270u, 35u));
        stretchBtn->setOnClick([this]() {
            if (image_) {
                image_->setScaleMode(Image::ScaleMode::Stretch);
                updateStatus(L"Image mode: Stretch");
                std::println("Image scale mode: Stretch");
            }
        });
        imageModeSection->addChild(stretchBtn);
        
        sidebar->addChild(imageModeSection);
        
        root->addChild(sidebar);
        
        // ====================================================================
        // MAIN CONTENT (Flex)
        // ====================================================================
        auto content = createFlexColumn(0, 0);
        content->setLayoutWeight(1.0f);
        content->setBackgroundColor(colors::White);
        
        // Header
        auto header = std::make_shared<Label>(L"Phase 3 Features Demo");
        header->setLayoutWeight(0.0f);
        header->setRect(Rect(0, 0, 600u, 60u));
        header->setBackgroundColor(Color(52, 152, 219));
        header->setTextColor(colors::White);
        header->setFontSize(20.0f);
        header->setFontBold(true);
        header->setAlignment(Label::Alignment::Center);
        header->setVerticalAlignment(Label::VerticalAlignment::Middle);
        content->addChild(header);
        
        // ScrollView with image and info
        auto scrollView = std::make_shared<ScrollView>();
        scrollView->setLayoutWeight(1.0f);
        scrollView->setBackgroundColor(Color(236, 240, 241));
        
        // ScrollView content
        auto scrollContent = createFlexColumn(20, 20);
        scrollContent->setBackgroundColor(colors::Transparent);
        
        // Image widget (placeholder - users should provide their own image)
        image_ = std::make_shared<Image>(L"test_image.png");
        image_->setLayoutWeight(0.0f);
        image_->setRect(Rect(0, 0, 560u, 400u));
        image_->setBackgroundColor(Color(200, 200, 200));
        image_->setScaleMode(Image::ScaleMode::Fit);
        scrollContent->addChild(image_);
        
        // Info text
        auto infoText = std::make_shared<Label>(
            L"Image Widget with WIC Support\n\n"
            L"• Supports PNG, JPEG, BMP, and more formats\n"
            L"• Three scale modes: Fit, Fill, Stretch\n"
            L"• Hardware-accelerated rendering via Direct2D\n\n"
            L"ScrollView Features:\n"
            L"• Draggable scrollbar thumbs\n"
            L"• Click on track to jump\n"
            L"• Mouse wheel scrolling\n"
            L"• Smooth content translation\n\n"
            L"To test the image, replace 'test_image.png' with your own image file."
        );
        infoText->setLayoutWeight(0.0f);
        infoText->setRect(Rect(0, 0, 560u, 300u));
        infoText->setBackgroundColor(colors::White);
        infoText->setTextColor(Color(44, 62, 80));
        infoText->setFontSize(14.0f);
        infoText->setPadding(20);
        infoText->setBorder(Color(189, 195, 199), 1.0f);
        scrollContent->addChild(infoText);
        
        // Add some test content to make scrolling meaningful
        for (int i = 0; i < 5; ++i) {
            auto testLabel = std::make_shared<Label>(
                std::format(L"Test Content Block #{}\n"
                           L"This is additional content to demonstrate scrolling behavior.", i + 1)
            );
            testLabel->setLayoutWeight(0.0f);
            testLabel->setRect(Rect(0, 0, 560u, 80u));
            testLabel->setBackgroundColor(i % 2 == 0 ? Color(250, 250, 250) : colors::White);
            testLabel->setPadding(15);
            testLabel->setBorder(Color(220, 220, 220), 1.0f);
            scrollContent->addChild(testLabel);
        }
        
        // Calculate content height
        uint32_t contentHeight = 400 + 300 + 80 * 5 + 20 * 7 + 40;  // Images + labels + spacing + padding
        scrollContent->setRect(Rect(0, 0, 560u, contentHeight));
        
        scrollView->setContent(scrollContent);
        content->addChild(scrollView);
        
        // Status bar
        statusLabel_ = std::make_shared<Label>(L"Ready. Try the checkboxes and image controls!");
        statusLabel_->setLayoutWeight(0.0f);
        statusLabel_->setRect(Rect(0, 0, 600u, 40u));
        statusLabel_->setBackgroundColor(Color(44, 62, 80));
        statusLabel_->setTextColor(colors::White);
        statusLabel_->setPadding(10);
        statusLabel_->setVerticalAlignment(Label::VerticalAlignment::Middle);
        content->addChild(statusLabel_);
        
        root->addChild(content);
        
        // Set root widget
        window->setRootWidget(root);
    }
    
    void updateStatus(const std::wstring& message) {
        if (statusLabel_) {
            try {
                statusLabel_->setText(message);
            } catch (...) {
                // Swallow exceptions
            }
        }
    }
};

// ============================================================================
// MAIN
// ============================================================================

int main() {
    Phase3Demo demo;
    demo.run();
    return 0;
}