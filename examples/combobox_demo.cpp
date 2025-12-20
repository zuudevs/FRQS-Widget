// examples/checkbox_demo.cpp - Comprehensive CheckBox Demo
#include "frqs-widget.hpp"
#include "widget/container.hpp"
#include "widget/checkbox.hpp"
#include "widget/label.hpp"
#include "widget/button.hpp"
#include <print>
#include <format>

using namespace frqs;
using namespace frqs::widget;

// ============================================================================
// CHECKBOX DEMO APPLICATION
// ============================================================================

class CheckBoxDemo {
private:
    std::shared_ptr<Label> statusLabel_;
    
    // Checkboxes for different features
    std::shared_ptr<CheckBox> cbSound_;
    std::shared_ptr<CheckBox> cbMusic_;
    std::shared_ptr<CheckBox> cbVibration_;
    std::shared_ptr<CheckBox> cbNotifications_;
    std::shared_ptr<CheckBox> cbAutoSave_;
    std::shared_ptr<CheckBox> cbDarkMode_;
    
    // Result display
    std::shared_ptr<Label> resultLabel_;

public:
    void run() {
        try {
            std::println("=== FRQS-Widget: CheckBox Demo ===");
            std::println("");
            std::println("Features Demonstrated:");
            std::println("  • Basic checkbox with text labels");
            std::println("  • Toggle on/off states");
            std::println("  • Change callbacks");
            std::println("  • Enable/disable states");
            std::println("  • Visual styling");
            std::println("");
            
            auto& app = Application::instance();
            app.initialize();
            
            // Create window
            WindowParams params;
            params.title = L"CheckBox Demo - Settings Panel";
            params.size = Size(700u, 600u);
            params.position = Point(100, 100);
            
            auto window = app.createWindow(params);
            
            // Build UI
            buildUI(window);
            
            window->show();
            
            std::println("✓ Demo window created");
            std::println("");
            std::println("Try:");
            std::println("  - Click checkboxes to toggle");
            std::println("  - Watch status updates");
            std::println("  - Click 'Apply Settings' to see current state");
            std::println("  - Click 'Reset to Defaults' to restore");
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
        auto root = createFlexRow(0, 0);
        root->setRect(window->getClientRect());
        root->setBackgroundColor(Color(245, 245, 245));
        
        // ====================================================================
        // LEFT PANEL - Settings
        // ====================================================================
        auto leftPanel = createFlexColumn(15, 20);
        leftPanel->setLayoutWeight(1.0f);
        leftPanel->setBackgroundColor(colors::White);
        
        // Title
        auto title = std::make_shared<Label>(L"⚙️ Application Settings");
        title->setLayoutWeight(0.0f);
        title->setRect(Rect(0, 0, 400u, 50u));
        title->setFontSize(20.0f);
        title->setFontBold(true);
        title->setTextColor(Color(44, 62, 80));
        leftPanel->addChild(title);
        
        // Audio Settings Section
        auto audioSection = createSection(L"🔊 Audio Settings");
        
        cbSound_ = std::make_shared<CheckBox>(L"Enable Sound Effects");
        cbSound_->setChecked(true);
        cbSound_->setRect(Rect(0, 0, 350u, 35u));
        cbSound_->setOnChanged([this](bool checked) {
            updateStatus(std::format(L"Sound effects: {}", checked ? L"ON" : L"OFF"));
        });
        audioSection->addChild(cbSound_);
        
        cbMusic_ = std::make_shared<CheckBox>(L"Enable Background Music");
        cbMusic_->setChecked(true);
        cbMusic_->setRect(Rect(0, 0, 350u, 35u));
        cbMusic_->setOnChanged([this](bool checked) {
            updateStatus(std::format(L"Background music: {}", checked ? L"ON" : L"OFF"));
        });
        audioSection->addChild(cbMusic_);
        
        cbVibration_ = std::make_shared<CheckBox>(L"Enable Vibration");
        cbVibration_->setChecked(false);
        cbVibration_->setRect(Rect(0, 0, 350u, 35u));
        cbVibration_->setOnChanged([this](bool checked) {
            updateStatus(std::format(L"Vibration: {}", checked ? L"ON" : L"OFF"));
        });
        audioSection->addChild(cbVibration_);
        
        leftPanel->addChild(audioSection);
        
        // Notifications Section
        auto notifSection = createSection(L"🔔 Notifications");
        
        cbNotifications_ = std::make_shared<CheckBox>(L"Show Notifications");
        cbNotifications_->setChecked(true);
        cbNotifications_->setRect(Rect(0, 0, 350u, 35u));
        cbNotifications_->setOnChanged([this](bool checked) {
            updateStatus(std::format(L"Notifications: {}", checked ? L"ON" : L"OFF"));
        });
        notifSection->addChild(cbNotifications_);
        
        leftPanel->addChild(notifSection);
        
        // General Settings Section
        auto generalSection = createSection(L"⚡ General");
        
        cbAutoSave_ = std::make_shared<CheckBox>(L"Auto-save Progress");
        cbAutoSave_->setChecked(true);
        cbAutoSave_->setRect(Rect(0, 0, 350u, 35u));
        cbAutoSave_->setOnChanged([this](bool checked) {
            updateStatus(std::format(L"Auto-save: {}", checked ? L"ON" : L"OFF"));
        });
        generalSection->addChild(cbAutoSave_);
        
        cbDarkMode_ = std::make_shared<CheckBox>(L"Dark Mode (Coming Soon)");
        cbDarkMode_->setChecked(false);
        cbDarkMode_->setEnabled(false);  // Disabled example
        cbDarkMode_->setRect(Rect(0, 0, 350u, 35u));
        generalSection->addChild(cbDarkMode_);
        
        leftPanel->addChild(generalSection);
        
        // Action Buttons
        auto buttonRow = createFlexRow(10, 0);
        buttonRow->setLayoutWeight(0.0f);
        
        auto btnApply = std::make_shared<Button>(L"Apply Settings");
        btnApply->setNormalColor(Color(46, 204, 113));
        btnApply->setHoverColor(Color(39, 174, 96));
        btnApply->setLayoutWeight(1.0f);
        btnApply->setOnClick([this]() {
            applySettings();
        });
        buttonRow->addChild(btnApply);
        
        auto btnReset = std::make_shared<Button>(L"Reset to Defaults");
        btnReset->setNormalColor(Color(149, 165, 166));
        btnReset->setHoverColor(Color(127, 140, 141));
        btnReset->setLayoutWeight(1.0f);
        btnReset->setOnClick([this]() {
            resetToDefaults();
        });
        buttonRow->addChild(btnReset);
        
        leftPanel->addChild(buttonRow);
        
        // Status Label
        statusLabel_ = std::make_shared<Label>(L"Ready. Change settings to see updates.");
        statusLabel_->setLayoutWeight(0.0f);
        statusLabel_->setRect(Rect(0, 0, 400u, 40u));
        statusLabel_->setBackgroundColor(Color(52, 152, 219));
        statusLabel_->setTextColor(colors::White);
        statusLabel_->setPadding(10);
        statusLabel_->setVerticalAlignment(Label::VerticalAlignment::Middle);
        leftPanel->addChild(statusLabel_);
        
        root->addChild(leftPanel);
        
        // ====================================================================
        // RIGHT PANEL - Current State
        // ====================================================================
        auto rightPanel = createFlexColumn(10, 15);
        rightPanel->setLayoutWeight(0.0f);
        rightPanel->setRect(Rect(0, 0, 280u, 600u));
        rightPanel->setBackgroundColor(Color(44, 62, 80));
        
        auto rightTitle = std::make_shared<Label>(L"Current State");
        rightTitle->setLayoutWeight(0.0f);
        rightTitle->setRect(Rect(0, 0, 250u, 40u));
        rightTitle->setTextColor(colors::White);
        rightTitle->setFontSize(16.0f);
        rightTitle->setFontBold(true);
        rightTitle->setAlignment(Label::Alignment::Center);
        rightPanel->addChild(rightTitle);
        
        resultLabel_ = std::make_shared<Label>(getSettingsText());
        resultLabel_->setLayoutWeight(1.0f);
        resultLabel_->setTextColor(Color(236, 240, 241));
        resultLabel_->setBackgroundColor(Color(52, 73, 94));
        resultLabel_->setPadding(15);
        resultLabel_->setAlignment(Label::Alignment::Left);
        resultLabel_->setVerticalAlignment(Label::VerticalAlignment::Top);
        rightPanel->addChild(resultLabel_);
        
        root->addChild(rightPanel);
        
        // Set root
        window->setRootWidget(root);
    }
    
    std::shared_ptr<Container> createSection(const std::wstring& title) {
        auto section = createFlexColumn(8, 15);
        section->setLayoutWeight(0.0f);
        section->setBackgroundColor(Color(249, 249, 249));
        section->setBorder(Color(220, 220, 220), 1.0f);
        
        auto sectionTitle = std::make_shared<Label>(title);
        sectionTitle->setLayoutWeight(0.0f);
        sectionTitle->setRect(Rect(0, 0, 400u, 30u));
        sectionTitle->setFontSize(14.0f);
        sectionTitle->setFontBold(true);
        sectionTitle->setTextColor(Color(52, 73, 94));
        section->addChild(sectionTitle);
        
        return section;
    }
    
    void updateStatus(const std::wstring& message) {
        if (statusLabel_) {
            statusLabel_->setText(message);
        }
        std::println("Status: {}", std::string(message.begin(), message.end()));
    }
    
    std::wstring getSettingsText() {
        std::wstring text = L"AUDIO:\n";
        text += std::format(L"  Sound Effects: {}\n", cbSound_->isChecked() ? L"✓ ON" : L"✗ OFF");
        text += std::format(L"  Music: {}\n", cbMusic_->isChecked() ? L"✓ ON" : L"✗ OFF");
        text += std::format(L"  Vibration: {}\n\n", cbVibration_->isChecked() ? L"✓ ON" : L"✗ OFF");
        
        text += L"NOTIFICATIONS:\n";
        text += std::format(L"  Enabled: {}\n\n", cbNotifications_->isChecked() ? L"✓ ON" : L"✗ OFF");
        
        text += L"GENERAL:\n";
        text += std::format(L"  Auto-save: {}\n", cbAutoSave_->isChecked() ? L"✓ ON" : L"✗ OFF");
        text += std::format(L"  Dark Mode: {}\n", cbDarkMode_->isChecked() ? L"✓ ON" : L"✗ OFF (disabled)");
        
        return text;
    }
    
    void applySettings() {
        if (resultLabel_) {
            resultLabel_->setText(getSettingsText());
        }
        updateStatus(L"✓ Settings applied successfully!");
        
        std::println("=== Settings Applied ===");
        std::println("Sound: {}", cbSound_->isChecked() ? "ON" : "OFF");
        std::println("Music: {}", cbMusic_->isChecked() ? "ON" : "OFF");
        std::println("Vibration: {}", cbVibration_->isChecked() ? "ON" : "OFF");
        std::println("Notifications: {}", cbNotifications_->isChecked() ? "ON" : "OFF");
        std::println("Auto-save: {}", cbAutoSave_->isChecked() ? "ON" : "OFF");
        std::println("========================");
    }
    
    void resetToDefaults() {
        cbSound_->setChecked(true);
        cbMusic_->setChecked(true);
        cbVibration_->setChecked(false);
        cbNotifications_->setChecked(true);
        cbAutoSave_->setChecked(true);
        
        if (resultLabel_) {
            resultLabel_->setText(getSettingsText());
        }
        
        updateStatus(L"🔄 Reset to default settings");
        std::println("Settings reset to defaults");
    }
};

// ============================================================================
// MAIN
// ============================================================================

int main() {
    CheckBoxDemo demo;
    demo.run();
    return 0;
}