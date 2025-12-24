// examples/comprehensive_dashboard.cpp - Full-Featured Dashboard

/**
 * @file dashboard_demo.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2025-12-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "frqs-widget.hpp"
#include "widget/container.hpp"
#include "widget/label.hpp"
#include "widget/button.hpp"
#include "widget/slider.hpp"
#include "widget/checkbox.hpp"
#include "widget/text_input.hpp"
#include "widget/scroll_view.hpp"
#include <print>
#include <format>

using namespace frqs;
using namespace frqs::widget;

class Dashboard {
private:
    // UI References
    std::shared_ptr<Label> statusBar_;
    std::shared_ptr<Label> systemStatus_;
    std::shared_ptr<Label> cpuLabel_;
    std::shared_ptr<Label> memoryLabel_;
    std::shared_ptr<Slider> volumeSlider_;
    std::shared_ptr<Slider> brightnessSlider_;
    std::shared_ptr<CheckBox> wifiCheck_;
    std::shared_ptr<CheckBox> bluetoothCheck_;
    std::shared_ptr<CheckBox> notificationsCheck_;
    std::shared_ptr<TextInput> searchInput_;
    
    // State
    bool systemRunning_ = false;
    int cpuUsage_ = 45;
    int memoryUsage_ = 62;

public:
    void run() {
        std::println("=== FRQS Comprehensive Dashboard ===");
        
        auto& app = Application::instance();
        app.initialize();
        
        WindowParams params;
        params.title = L"System Control Dashboard - All Widgets";
        params.size = Size(1400u, 900u);
        params.position = Point(50, 50);
        
        auto window = app.createWindow(params);
        
        buildUI(window);
        
        window->show();
        
        std::println("✓ Dashboard loaded with all features:");
        std::println("  - System monitoring");
        std::println("  - Sliders (Volume, Brightness)");
        std::println("  - Checkboxes (WiFi, Bluetooth, Notifications)");
        std::println("  - Text input (Search)");
        std::println("  - Buttons (Start/Stop, Actions)");
        std::println("  - ScrollView for logs");
        std::println("  - Real-time status updates");
        
        app.run();
    }

private:
    void buildUI(std::shared_ptr<core::Window> window) {
        // Root: Main layout
        auto root = createFlexRow(0, 0);
        root->setRect(window->getClientRect());
        root->setBackgroundColor(Color(240, 242, 245));
        
        // LEFT SIDEBAR
        buildSidebar(root);
        
        // MAIN CONTENT
        buildMainContent(root);
        
        // RIGHT PANEL
        buildRightPanel(root);
        
        window->setRootWidget(root);
    }
    
    void buildSidebar(std::shared_ptr<Container> root) {
        auto sidebar = createFlexColumn(5, 15);
        sidebar->setLayoutWeight(0.0f);
        sidebar->setRect(Rect(0, 0, 200u, 900u));
        sidebar->setBackgroundColor(Color(44, 62, 80));
        
        // Logo
        auto logo = std::make_shared<Label>(L"🖥️ SYSTEM\nCONTROL");
        logo->setLayoutWeight(0.0f);
        logo->setRect(Rect(0, 0, 170u, 80u));
        logo->setFontSize(18.0f);
        logo->setFontBold(true);
        logo->setTextColor(colors::White);
        logo->setAlignment(Label::Alignment::Center);
        sidebar->addChild(logo);
        
        // Navigation Buttons
        auto navButtons = std::vector<std::wstring>{
            L"📊 Dashboard",
            L"⚙️ Settings",
            L"📁 Files",
            L"👤 Profile",
            L"🔔 Alerts",
            L"📈 Analytics"
        };
        
        for (const auto& text : navButtons) {
            auto btn = std::make_shared<Button>(text);
            btn->setLayoutWeight(0.0f);
            btn->setRect(Rect(0, 0, 170u, 45u));
            btn->setNormalColor(Color(52, 73, 94));
            btn->setHoverColor(Color(41, 128, 185));
            btn->setOnClick([this, text]() {
                updateStatus(std::format(L"Navigated to {}", text));
            });
            sidebar->addChild(btn);
        }
        
        // Spacer
        auto spacer = std::make_shared<Widget>();
        spacer->setLayoutWeight(1.0f);
        spacer->setBackgroundColor(colors::Transparent);
        sidebar->addChild(spacer);
        
        // Power Button
        auto powerBtn = std::make_shared<Button>(L"⚡ Power");
        powerBtn->setLayoutWeight(0.0f);
        powerBtn->setRect(Rect(0, 0, 170u, 50u));
        powerBtn->setNormalColor(Color(231, 76, 60));
        powerBtn->setHoverColor(Color(192, 57, 43));
        powerBtn->setOnClick([this]() {
            updateStatus(L"⚠️ Power button pressed");
        });
        sidebar->addChild(powerBtn);
        
        root->addChild(sidebar);
    }
    
    void buildMainContent(std::shared_ptr<Container> root) {
        auto mainContent = createFlexColumn(20, 20);
        mainContent->setLayoutWeight(1.0f);
        
        // Header
        auto header = createFlexRow(15, 0);
        header->setLayoutWeight(0.0f);
        header->setRect(Rect(0, 0, 900u, 60u));
        
        auto title = std::make_shared<Label>(L"🎛️ Control Dashboard");
        title->setLayoutWeight(1.0f);
        title->setFontSize(24.0f);
        title->setFontBold(true);
        header->addChild(title);
        
        // Search Box
        searchInput_ = std::make_shared<TextInput>();
        searchInput_->setLayoutWeight(0.0f);
        searchInput_->setRect(Rect(0, 0, 250u, 40u));
        searchInput_->setPlaceholder(L"🔍 Search...");
        searchInput_->setOnEnter([this](const std::wstring& text) {
            updateStatus(std::format(L"Search: {}", text));
        });
        header->addChild(searchInput_);
        
        mainContent->addChild(header);
        
        // System Status Cards
        auto cardsRow = createFlexRow(15, 0);
        cardsRow->setLayoutWeight(0.0f);
        cardsRow->setRect(Rect(0, 0, 900u, 120u));
        
        // CPU Card
        auto cpuCard = createFlexColumn(5, 15);
        cpuCard->setLayoutWeight(1.0f);
        cpuCard->setBackgroundColor(colors::White);
        cpuCard->setBorder(Color(189, 195, 199), 1.0f);
        
        auto cpuTitle = std::make_shared<Label>(L"💻 CPU Usage");
        cpuTitle->setFontBold(true);
        cpuCard->addChild(cpuTitle);
        
        cpuLabel_ = std::make_shared<Label>(L"45%");
        cpuLabel_->setFontSize(32.0f);
        cpuLabel_->setFontBold(true);
        cpuLabel_->setTextColor(Color(52, 152, 219));
        cpuLabel_->setAlignment(Label::Alignment::Center);
        cpuCard->addChild(cpuLabel_);
        
        cardsRow->addChild(cpuCard);
        
        // Memory Card
        auto memCard = createFlexColumn(5, 15);
        memCard->setLayoutWeight(1.0f);
        memCard->setBackgroundColor(colors::White);
        memCard->setBorder(Color(189, 195, 199), 1.0f);
        
        auto memTitle = std::make_shared<Label>(L"🧠 Memory");
        memTitle->setFontBold(true);
        memCard->addChild(memTitle);
        
        memoryLabel_ = std::make_shared<Label>(L"62%");
        memoryLabel_->setFontSize(32.0f);
        memoryLabel_->setFontBold(true);
        memoryLabel_->setTextColor(Color(155, 89, 182));
        memoryLabel_->setAlignment(Label::Alignment::Center);
        memCard->addChild(memoryLabel_);
        
        cardsRow->addChild(memCard);
        
        // Status Card
        auto statusCard = createFlexColumn(5, 15);
        statusCard->setLayoutWeight(1.0f);
        statusCard->setBackgroundColor(colors::White);
        statusCard->setBorder(Color(189, 195, 199), 1.0f);
        
        auto statusTitle = std::make_shared<Label>(L"📡 Status");
        statusTitle->setFontBold(true);
        statusCard->addChild(statusTitle);
        
        systemStatus_ = std::make_shared<Label>(L"STOPPED");
        systemStatus_->setFontSize(20.0f);
        systemStatus_->setFontBold(true);
        systemStatus_->setTextColor(Color(231, 76, 60));
        systemStatus_->setAlignment(Label::Alignment::Center);
        statusCard->addChild(systemStatus_);
        
        cardsRow->addChild(statusCard);
        
        mainContent->addChild(cardsRow);
        
        // Control Section
        auto controlSection = createFlexColumn(15, 20);
        controlSection->setLayoutWeight(0.0f);
        controlSection->setBackgroundColor(colors::White);
        controlSection->setBorder(Color(189, 195, 199), 1.0f);
        
        auto controlTitle = std::make_shared<Label>(L"🎚️ System Controls");
        controlTitle->setFontSize(18.0f);
        controlTitle->setFontBold(true);
        controlSection->addChild(controlTitle);
        
        // Volume Control
        auto volumeRow = createFlexRow(15, 0);
        volumeRow->setLayoutWeight(0.0f);
        
        auto volumeLabel = std::make_shared<Label>(L"🔊 Volume: 50");
        volumeLabel->setLayoutWeight(0.0f);
        volumeLabel->setRect(Rect(0, 0, 150u, 40u));
        volumeRow->addChild(volumeLabel);
        
        volumeSlider_ = std::make_shared<Slider>(Slider::Orientation::Horizontal);
        volumeSlider_->setLayoutWeight(1.0f);
        volumeSlider_->setRange(0, 100);
        volumeSlider_->setValue(50);
        volumeSlider_->setOnValueChanged([this, volumeLabel](double value) {
            volumeLabel->setText(std::format(L"🔊 Volume: {}", static_cast<int>(value)));
        });
        volumeRow->addChild(volumeSlider_);
        
        controlSection->addChild(volumeRow);
        
        // Brightness Control
        auto brightnessRow = createFlexRow(15, 0);
        brightnessRow->setLayoutWeight(0.0f);
        
        auto brightnessLabel = std::make_shared<Label>(L"💡 Brightness: 75");
        brightnessLabel->setLayoutWeight(0.0f);
        brightnessLabel->setRect(Rect(0, 0, 150u, 40u));
        brightnessRow->addChild(brightnessLabel);
        
        brightnessSlider_ = std::make_shared<Slider>(Slider::Orientation::Horizontal);
        brightnessSlider_->setLayoutWeight(1.0f);
        brightnessSlider_->setRange(0, 100);
        brightnessSlider_->setValue(75);
        brightnessSlider_->setOnValueChanged([this, brightnessLabel](double value) {
            brightnessLabel->setText(std::format(L"💡 Brightness: {}", static_cast<int>(value)));
        });
        brightnessRow->addChild(brightnessSlider_);
        
        controlSection->addChild(brightnessRow);
        
        mainContent->addChild(controlSection);
        
        // Action Buttons
        auto actionRow = createFlexRow(15, 0);
        actionRow->setLayoutWeight(0.0f);
        
        auto startBtn = std::make_shared<Button>(L"▶️ Start System");
        startBtn->setLayoutWeight(1.0f);
        startBtn->setNormalColor(Color(46, 204, 113));
        startBtn->setOnClick([this]() {
            systemRunning_ = true;
            systemStatus_->setText(L"RUNNING");
            systemStatus_->setTextColor(Color(46, 204, 113));
            updateStatus(L"✓ System started successfully");
        });
        actionRow->addChild(startBtn);
        
        auto stopBtn = std::make_shared<Button>(L"⏹️ Stop System");
        stopBtn->setLayoutWeight(1.0f);
        stopBtn->setNormalColor(Color(231, 76, 60));
        stopBtn->setOnClick([this]() {
            systemRunning_ = false;
            systemStatus_->setText(L"STOPPED");
            systemStatus_->setTextColor(Color(231, 76, 60));
            updateStatus(L"⏹️ System stopped");
        });
        actionRow->addChild(stopBtn);
        
        auto restartBtn = std::make_shared<Button>(L"🔄 Restart");
        restartBtn->setLayoutWeight(1.0f);
        restartBtn->setNormalColor(Color(241, 196, 15));
        restartBtn->setOnClick([this]() {
            updateStatus(L"🔄 System restarting...");
        });
        actionRow->addChild(restartBtn);
        
        mainContent->addChild(actionRow);
        
        root->addChild(mainContent);
    }
    
    void buildRightPanel(std::shared_ptr<Container> root) {
        auto rightPanel = createFlexColumn(15, 15);
        rightPanel->setLayoutWeight(0.0f);
        rightPanel->setRect(Rect(0, 0, 280u, 900u));
        rightPanel->setBackgroundColor(colors::White);
        rightPanel->setBorder(Color(189, 195, 199), 1.0f);
        
        // Quick Settings
        auto settingsTitle = std::make_shared<Label>(L"⚙️ Quick Settings");
        settingsTitle->setFontSize(16.0f);
        settingsTitle->setFontBold(true);
        rightPanel->addChild(settingsTitle);
        
        // WiFi Toggle
        wifiCheck_ = std::make_shared<CheckBox>(L"WiFi");
        wifiCheck_->setChecked(true);
        wifiCheck_->setRect(Rect(0, 0, 250u, 35u));
        wifiCheck_->setOnChanged([this](bool checked) {
            updateStatus(std::format(L"WiFi: {}", checked ? L"ON" : L"OFF"));
        });
        rightPanel->addChild(wifiCheck_);
        
        // Bluetooth Toggle
        bluetoothCheck_ = std::make_shared<CheckBox>(L"Bluetooth");
        bluetoothCheck_->setChecked(false);
        bluetoothCheck_->setRect(Rect(0, 0, 250u, 35u));
        bluetoothCheck_->setOnChanged([this](bool checked) {
            updateStatus(std::format(L"Bluetooth: {}", checked ? L"ON" : L"OFF"));
        });
        rightPanel->addChild(bluetoothCheck_);
        
        // Notifications Toggle
        notificationsCheck_ = std::make_shared<CheckBox>(L"Notifications");
        notificationsCheck_->setChecked(true);
        notificationsCheck_->setRect(Rect(0, 0, 250u, 35u));
        notificationsCheck_->setOnChanged([this](bool checked) {
            updateStatus(std::format(L"Notifications: {}", checked ? L"ON" : L"OFF"));
        });
        rightPanel->addChild(notificationsCheck_);
        
        // Activity Log
        auto logTitle = std::make_shared<Label>(L"📋 Activity Log");
        logTitle->setLayoutWeight(0.0f);
        logTitle->setFontBold(true);
        rightPanel->addChild(logTitle);
        
        // ScrollView for logs
        auto scrollView = std::make_shared<ScrollView>();
        scrollView->setLayoutWeight(1.0f);
        scrollView->setBackgroundColor(Color(236, 240, 241));
        
        auto logContent = createFlexColumn(5, 10);
        logContent->setBackgroundColor(Color(236, 240, 241));
        
        auto logs = std::vector<std::wstring>{
            L"[10:23] System initialized",
            L"[10:24] WiFi connected",
            L"[10:25] User logged in",
            L"[10:26] Dashboard loaded",
            L"[10:27] Volume adjusted",
            L"[10:28] Brightness set",
            L"[10:29] All systems nominal",
            L"[10:30] Standing by..."
        };
        
        uint32_t logHeight = static_cast<uint32_t>(logs.size() * 35);
        logContent->setRect(Rect(0, 0, 240u, logHeight));
        
        for (const auto& log : logs) {
            auto logEntry = std::make_shared<Label>(log);
            logEntry->setLayoutWeight(0.0f);
            logEntry->setRect(Rect(0, 0, 240u, 30u));
            logEntry->setBackgroundColor(colors::White);
            logEntry->setPadding(5);
            logContent->addChild(logEntry);
        }
        
        scrollView->setContent(logContent);
        rightPanel->addChild(scrollView);
        
        // Status Bar (moved to bottom)
        statusBar_ = std::make_shared<Label>(L"Ready");
        statusBar_->setLayoutWeight(0.0f);
        statusBar_->setRect(Rect(0, 0, 250u, 35u));
        statusBar_->setBackgroundColor(Color(52, 152, 219));
        statusBar_->setTextColor(colors::White);
        statusBar_->setPadding(8);
        rightPanel->addChild(statusBar_);
        
        root->addChild(rightPanel);
    }
    
    void updateStatus(const std::wstring& message) {
        if (statusBar_) {
            statusBar_->setText(message);
        }
        std::println("Status: {}", std::string(message.begin(), message.end()));
    }
};

int main() {
    Dashboard dashboard;
    dashboard.run();
    return 0;
}