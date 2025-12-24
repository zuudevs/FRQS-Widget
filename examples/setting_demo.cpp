// examples/comprehensive_settings.cpp - Settings & Configuration Panel
#include "frqs-widget.hpp"
#include "widget/container.hpp"
#include "widget/label.hpp"
#include "widget/button.hpp"
#include "widget/text_input.hpp"
#include "widget/slider.hpp"
#include "widget/checkbox.hpp"
#include "widget/combobox.hpp"
#include "widget/list_adapter.hpp"
#include "widget/scroll_view.hpp"
#include <print>
#include <format>

using namespace frqs;
using namespace frqs::widget;

// String adapter for ComboBox
class StringAdapter : public IListAdapter {
private:
    std::vector<std::wstring> items_;
    
public:
    explicit StringAdapter(std::vector<std::wstring> items)
        : items_(std::move(items)) {}
    
    size_t getCount() const override { return items_.size(); }
    
    std::shared_ptr<IWidget> createView(size_t) override {
        auto label = std::make_shared<Label>();
        label->setAlignment(Label::Alignment::Left);
        label->setVerticalAlignment(Label::VerticalAlignment::Middle);
        label->setPadding(10);
        return label;
    }
    
    void updateView(size_t index, IWidget* view) override {
        if (index >= items_.size()) return;
        auto* label = dynamic_cast<Label*>(view);
        if (!label) return;
        label->setText(items_[index]);
        label->setBackgroundColor(index % 2 == 0 ? colors::White : Color(250, 250, 250));
    }
};

class SettingsPanel {
private:
    // UI References
    std::shared_ptr<Label> statusLabel_;
    std::shared_ptr<TextInput> usernameInput_;
    std::shared_ptr<TextInput> emailInput_;
    std::shared_ptr<TextInput> passwordInput_;
    std::shared_ptr<ComboBox> themeCombo_;
    std::shared_ptr<ComboBox> languageCombo_;
    std::shared_ptr<ComboBox> regionCombo_;
    std::shared_ptr<Slider> fontSizeSlider_;
    std::shared_ptr<CheckBox> darkModeCheck_;
    std::shared_ptr<CheckBox> autoSaveCheck_;
    std::shared_ptr<CheckBox> notificationsCheck_;
    std::shared_ptr<CheckBox> soundCheck_;
    std::shared_ptr<Label> previewLabel_;

public:
    void run() {
        std::println("=== FRQS Settings & Configuration Panel ===");
        
        auto& app = Application::instance();
        app.initialize();
        
        WindowParams params;
        params.title = L"Settings & Configuration - Complete Form Demo";
        params.size = Size(1200u, 800u);
        params.position = Point(100, 50);
        
        auto window = app.createWindow(params);
        
        buildUI(window);
        
        window->show();
        
        std::println("✓ Settings panel loaded with:");
        std::println("  - Text inputs (Username, Email, Password)");
        std::println("  - ComboBoxes (Theme, Language, Region)");
        std::println("  - Sliders (Font size)");
        std::println("  - Checkboxes (Multiple settings)");
        std::println("  - Validation & preview");
        std::println("  - Save/Reset/Export functionality");
        
        app.run();
    }

private:
    void buildUI(std::shared_ptr<core::Window> window) {
        auto root = createFlexRow(0, 0);
        root->setRect(window->getClientRect());
        root->setBackgroundColor(Color(245, 245, 250));
        
        // Navigation Sidebar
        buildNavigation(root);
        
        // Main Settings Content
        buildMainSettings(root);
        
        // Preview Panel
        buildPreview(root);
        
        window->setRootWidget(root);
    }
    
    void buildNavigation(std::shared_ptr<Container> root) {
        auto nav = createFlexColumn(5, 15);
        nav->setLayoutWeight(0.0f);
        nav->setRect(Rect(0, 0, 200u, 800u));
        nav->setBackgroundColor(Color(52, 73, 94));
        
        // Title
        auto title = std::make_shared<Label>(L"⚙️\nSettings");
        title->setLayoutWeight(0.0f);
        title->setRect(Rect(0, 0, 170u, 80u));
        title->setFontSize(20.0f);
        title->setFontBold(true);
        title->setTextColor(colors::White);
        title->setAlignment(Label::Alignment::Center);
        nav->addChild(title);
        
        // Navigation Items
        auto navItems = std::vector<std::wstring>{
            L"👤 Account",
            L"🎨 Appearance",
            L"🔔 Notifications",
            L"🌍 Language",
            L"🔒 Privacy",
            L"🔧 Advanced"
        };
        
        for (const auto& item : navItems) {
            auto btn = std::make_shared<Button>(item);
            btn->setLayoutWeight(0.0f);
            btn->setRect(Rect(0, 0, 170u, 45u));
            btn->setNormalColor(Color(44, 62, 80));
            btn->setHoverColor(Color(41, 128, 185));
            btn->setOnClick([this, item]() {
                updateStatus(std::format(L"Viewing: {}", item));
            });
            nav->addChild(btn);
        }
        
        root->addChild(nav);
    }
    
    void buildMainSettings(std::shared_ptr<Container> root) {
        // ScrollView for settings
        auto scrollView = std::make_shared<ScrollView>();
        scrollView->setLayoutWeight(1.0f);
        scrollView->setBackgroundColor(Color(245, 245, 250));
        
        auto content = createFlexColumn(20, 20);
        content->setBackgroundColor(Color(245, 245, 250));
        
        // Header
        auto header = std::make_shared<Label>(L"⚙️ General Settings");
        header->setLayoutWeight(0.0f);
        header->setRect(Rect(0, 0, 700u, 50u));
        header->setFontSize(24.0f);
        header->setFontBold(true);
        content->addChild(header);
        
        // Account Section
        content->addChild(createSection(L"👤 Account Information"));
        
        // Username
        auto usernameRow = createFlexRow(15, 0);
        usernameRow->setLayoutWeight(0.0f);
        
        auto usernameLabel = std::make_shared<Label>(L"Username:");
        usernameLabel->setLayoutWeight(0.0f);
        usernameLabel->setRect(Rect(0, 0, 120u, 40u));
        usernameRow->addChild(usernameLabel);
        
        usernameInput_ = std::make_shared<TextInput>();
        usernameInput_->setLayoutWeight(1.0f);
        usernameInput_->setPlaceholder(L"Enter username...");
        usernameInput_->setOnTextChanged([this](const std::wstring& text) {
            if (text.length() < 3) {
                updateStatus(L"⚠️ Username too short (min 3 chars)");
            }
        });
        usernameRow->addChild(usernameInput_);
        
        content->addChild(usernameRow);
        
        // Email
        auto emailRow = createFlexRow(15, 0);
        emailRow->setLayoutWeight(0.0f);
        
        auto emailLabel = std::make_shared<Label>(L"Email:");
        emailLabel->setLayoutWeight(0.0f);
        emailLabel->setRect(Rect(0, 0, 120u, 40u));
        emailRow->addChild(emailLabel);
        
        emailInput_ = std::make_shared<TextInput>();
        emailInput_->setLayoutWeight(1.0f);
        emailInput_->setPlaceholder(L"user@example.com");
        emailInput_->setOnEnter([this](const std::wstring& text) {
            updateStatus(std::format(L"Email set: {}", text));
        });
        emailRow->addChild(emailInput_);
        
        content->addChild(emailRow);
        
        // Password
        auto passwordRow = createFlexRow(15, 0);
        passwordRow->setLayoutWeight(0.0f);
        
        auto passwordLabel = std::make_shared<Label>(L"Password:");
        passwordLabel->setLayoutWeight(0.0f);
        passwordLabel->setRect(Rect(0, 0, 120u, 40u));
        passwordRow->addChild(passwordLabel);
        
        passwordInput_ = std::make_shared<TextInput>();
        passwordInput_->setLayoutWeight(1.0f);
        passwordInput_->setPlaceholder(L"Enter new password...");
        passwordRow->addChild(passwordInput_);
        
        content->addChild(passwordRow);
        
        // Appearance Section
        content->addChild(createSection(L"🎨 Appearance"));
        
        // Theme ComboBox
        auto themeRow = createFlexRow(15, 0);
        themeRow->setLayoutWeight(0.0f);
        
        auto themeLabel = std::make_shared<Label>(L"Theme:");
        themeLabel->setLayoutWeight(0.0f);
        themeLabel->setRect(Rect(0, 0, 120u, 40u));
        themeRow->addChild(themeLabel);
        
        themeCombo_ = std::make_shared<ComboBox>();
        themeCombo_->setLayoutWeight(1.0f);
        themeCombo_->setRect(Rect(0, 0, 400u, 40u));
        themeCombo_->setAdapter(std::make_shared<StringAdapter>(std::vector<std::wstring>{
            L"🌟 Light Theme",
            L"🌙 Dark Theme",
            L"🎨 Auto (System)",
            L"🌈 High Contrast"
        }));
        themeCombo_->setOnSelectionChanged([this](size_t idx) {
            updateStatus(std::format(L"Theme changed to option {}", idx + 1));
        });
        themeRow->addChild(themeCombo_);
        
        content->addChild(themeRow);
        
        // Font Size Slider
        auto fontRow = createFlexRow(15, 0);
        fontRow->setLayoutWeight(0.0f);
        
        auto fontLabel = std::make_shared<Label>(L"Font Size: 14px");
        fontLabel->setLayoutWeight(0.0f);
        fontLabel->setRect(Rect(0, 0, 150u, 40u));
        fontRow->addChild(fontLabel);
        
        fontSizeSlider_ = std::make_shared<Slider>(Slider::Orientation::Horizontal);
        fontSizeSlider_->setLayoutWeight(1.0f);
        fontSizeSlider_->setRange(10, 24);
        fontSizeSlider_->setValue(14);
        fontSizeSlider_->setOnValueChanged([this, fontLabel](double value) {
            int size = static_cast<int>(value);
            fontLabel->setText(std::format(L"Font Size: {}px", size));
            if (previewLabel_) {
                previewLabel_->setFontSize(static_cast<float>(size));
            }
        });
        fontRow->addChild(fontSizeSlider_);
        
        content->addChild(fontRow);
        
        // Preferences Section
        content->addChild(createSection(L"🔧 Preferences"));
        
        // Dark Mode
        darkModeCheck_ = std::make_shared<CheckBox>(L"Enable Dark Mode");
        darkModeCheck_->setRect(Rect(0, 0, 400u, 35u));
        darkModeCheck_->setOnChanged([this](bool checked) {
            updateStatus(std::format(L"Dark Mode: {}", checked ? L"ON" : L"OFF"));
        });
        content->addChild(darkModeCheck_);
        
        // Auto Save
        autoSaveCheck_ = std::make_shared<CheckBox>(L"Auto-save Settings");
        autoSaveCheck_->setChecked(true);
        autoSaveCheck_->setRect(Rect(0, 0, 400u, 35u));
        autoSaveCheck_->setOnChanged([this](bool checked) {
            updateStatus(std::format(L"Auto-save: {}", checked ? L"ON" : L"OFF"));
        });
        content->addChild(autoSaveCheck_);
        
        // Notifications
        notificationsCheck_ = std::make_shared<CheckBox>(L"Enable Notifications");
        notificationsCheck_->setChecked(true);
        notificationsCheck_->setRect(Rect(0, 0, 400u, 35u));
        notificationsCheck_->setOnChanged([this](bool checked) {
            updateStatus(std::format(L"Notifications: {}", checked ? L"ON" : L"OFF"));
        });
        content->addChild(notificationsCheck_);
        
        // Sound Effects
        soundCheck_ = std::make_shared<CheckBox>(L"Sound Effects");
        soundCheck_->setChecked(true);
        soundCheck_->setRect(Rect(0, 0, 400u, 35u));
        soundCheck_->setOnChanged([this](bool checked) {
            updateStatus(std::format(L"Sound: {}", checked ? L"ON" : L"OFF"));
        });
        content->addChild(soundCheck_);
        
        // Localization Section
        content->addChild(createSection(L"🌍 Localization"));
        
        // Language ComboBox
        auto langRow = createFlexRow(15, 0);
        langRow->setLayoutWeight(0.0f);
        
        auto langLabel = std::make_shared<Label>(L"Language:");
        langLabel->setLayoutWeight(0.0f);
        langLabel->setRect(Rect(0, 0, 120u, 40u));
        langRow->addChild(langLabel);
        
        languageCombo_ = std::make_shared<ComboBox>();
        languageCombo_->setLayoutWeight(1.0f);
        languageCombo_->setRect(Rect(0, 0, 400u, 40u));
        languageCombo_->setAdapter(std::make_shared<StringAdapter>(std::vector<std::wstring>{
            L"🇺🇸 English",
            L"🇮🇩 Indonesian",
            L"🇯🇵 Japanese",
            L"🇨🇳 Chinese",
            L"🇪🇸 Spanish"
        }));
        langRow->addChild(languageCombo_);
        
        content->addChild(langRow);
        
        // Region ComboBox
        auto regionRow = createFlexRow(15, 0);
        regionRow->setLayoutWeight(0.0f);
        
        auto regionLabel = std::make_shared<Label>(L"Region:");
        regionLabel->setLayoutWeight(0.0f);
        regionLabel->setRect(Rect(0, 0, 120u, 40u));
        regionRow->addChild(regionLabel);
        
        regionCombo_ = std::make_shared<ComboBox>();
        regionCombo_->setLayoutWeight(1.0f);
        regionCombo_->setRect(Rect(0, 0, 400u, 40u));
        regionCombo_->setAdapter(std::make_shared<StringAdapter>(std::vector<std::wstring>{
            L"Asia/Jakarta",
            L"America/New_York",
            L"Europe/London",
            L"Asia/Tokyo"
        }));
        regionRow->addChild(regionCombo_);
        
        content->addChild(regionRow);
        
        // Action Buttons
        auto actionRow = createFlexRow(15, 0);
        actionRow->setLayoutWeight(0.0f);
        
        auto saveBtn = std::make_shared<Button>(L"💾 Save Settings");
        saveBtn->setLayoutWeight(1.0f);
        saveBtn->setNormalColor(Color(46, 204, 113));
        saveBtn->setOnClick([this]() {
            saveSettings();
        });
        actionRow->addChild(saveBtn);
        
        auto resetBtn = std::make_shared<Button>(L"🔄 Reset to Defaults");
        resetBtn->setLayoutWeight(1.0f);
        resetBtn->setNormalColor(Color(149, 165, 166));
        resetBtn->setOnClick([this]() {
            resetSettings();
        });
        actionRow->addChild(resetBtn);
        
        auto exportBtn = std::make_shared<Button>(L"📤 Export Config");
        exportBtn->setLayoutWeight(1.0f);
        exportBtn->setNormalColor(Color(52, 152, 219));
        exportBtn->setOnClick([this]() {
            updateStatus(L"📤 Configuration exported to config.json");
        });
        actionRow->addChild(exportBtn);
        
        content->addChild(actionRow);
        
        // Status Bar
        statusLabel_ = std::make_shared<Label>(L"Ready to configure");
        statusLabel_->setLayoutWeight(0.0f);
        statusLabel_->setRect(Rect(0, 0, 700u, 40u));
        statusLabel_->setBackgroundColor(Color(52, 152, 219));
        statusLabel_->setTextColor(colors::White);
        statusLabel_->setPadding(10);
        content->addChild(statusLabel_);
        
        // Set content size for scrolling
        content->setRect(Rect(0, 0, 700u, 1000u));
        scrollView->setContent(content);
        
        root->addChild(scrollView);
    }
    
    void buildPreview(std::shared_ptr<Container> root) {
        auto preview = createFlexColumn(15, 15);
        preview->setLayoutWeight(0.0f);
        preview->setRect(Rect(0, 0, 280u, 800u));
        preview->setBackgroundColor(colors::White);
        preview->setBorder(Color(189, 195, 199), 1.0f);
        
        // Title
        auto title = std::make_shared<Label>(L"👁️ Live Preview");
        title->setFontSize(18.0f);
        title->setFontBold(true);
        preview->addChild(title);
        
        // Preview Label
        previewLabel_ = std::make_shared<Label>(
            L"The quick brown fox jumps over the lazy dog.\n\n"
            L"This is a preview of your selected font size and theme.\n\n"
            L"Adjust settings to see changes in real-time."
        );
        previewLabel_->setLayoutWeight(1.0f);
        previewLabel_->setBackgroundColor(Color(236, 240, 241));
        previewLabel_->setPadding(15);
        preview->addChild(previewLabel_);
        
        // Info Panel
        auto info = std::make_shared<Label>(
            L"ℹ️ Configuration Info\n\n"
            L"• All changes are saved automatically\n"
            L"• Settings sync across devices\n"
            L"• Export for backup\n"
            L"• Reset anytime"
        );
        info->setLayoutWeight(0.0f);
        info->setBackgroundColor(Color(241, 196, 15));
        info->setPadding(15);
        preview->addChild(info);
        
        root->addChild(preview);
    }
    
    std::shared_ptr<Label> createSection(const std::wstring& title) {
        auto section = std::make_shared<Label>(title);
        section->setLayoutWeight(0.0f);
        section->setRect(Rect(0, 0, 700u, 40u));
        section->setFontSize(16.0f);
        section->setFontBold(true);
        section->setBackgroundColor(Color(52, 73, 94));
        section->setTextColor(colors::White);
        section->setPadding(10);
        return section;
    }
    
    void saveSettings() {
        std::wstring username = usernameInput_ ? usernameInput_->getText() : L"";
        std::wstring email = emailInput_ ? emailInput_->getText() : L"";
        
        if (username.empty()) {
            updateStatus(L"❌ Username is required!");
            return;
        }
        
        if (email.empty()) {
            updateStatus(L"❌ Email is required!");
            return;
        }
        
        updateStatus(L"✅ Settings saved successfully!");
        
        std::println("Settings saved:");
        std::println("  Username: {}", std::string(username.begin(), username.end()));
        std::println("  Email: {}", std::string(email.begin(), email.end()));
        std::println("  Dark Mode: {}", darkModeCheck_->isChecked() ? "ON" : "OFF");
        std::println("  Auto-save: {}", autoSaveCheck_->isChecked() ? "ON" : "OFF");
        std::println("  Notifications: {}", notificationsCheck_->isChecked() ? "ON" : "OFF");
    }
    
    void resetSettings() {
        if (usernameInput_) usernameInput_->clear();
        if (emailInput_) emailInput_->clear();
        if (passwordInput_) passwordInput_->clear();
        if (darkModeCheck_) darkModeCheck_->setChecked(false);
        if (autoSaveCheck_) autoSaveCheck_->setChecked(true);
        if (notificationsCheck_) notificationsCheck_->setChecked(true);
        if (soundCheck_) soundCheck_->setChecked(true);
        if (fontSizeSlider_) fontSizeSlider_->setValue(14);
        
        updateStatus(L"🔄 Settings reset to defaults");
    }
    
    void updateStatus(const std::wstring& message) {
        if (statusLabel_) {
            statusLabel_->setText(message);
        }
        std::println("Status: {}", std::string(message.begin(), message.end()));
    }
};

int main() {
    SettingsPanel panel;
    panel.run();
    return 0;
}