// examples/button_showcase.cpp - Comprehensive Button Demo

/**
 * @file button_demo.cpp
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
#include "widget/button.hpp"
#include "widget/label.hpp"
#include <print>

using namespace frqs;
using namespace frqs::widget;

int main() {
    std::println("=== FRQS Button Showcase ===");
    
    auto& app = Application::instance();
    app.initialize();
    
    WindowParams params;
    params.title = L"Button Showcase - All Styles & States";
    params.size = Size(900u, 700u);
    params.position = Point(100, 100);
    
    auto window = app.createWindow(params);
    
    // Root container
    auto root = createFlexColumn(20, 20);
    root->setRect(window->getClientRect());
    root->setBackgroundColor(Color(245, 245, 250));
    
    // Title
    auto title = std::make_shared<Label>(L"🎨 Button Showcase");
    title->setLayoutWeight(0.0f);
    title->setRect(Rect(0, 0, 860u, 60u));
    title->setFontSize(24.0f);
    title->setFontBold(true);
    title->setAlignment(Label::Alignment::Center);
    title->setBackgroundColor(Color(41, 128, 185));
    title->setTextColor(colors::White);
    root->addChild(title);
    
    // Status label
    auto statusLabel = std::make_shared<Label>(L"Click any button to see action");
    statusLabel->setLayoutWeight(0.0f);
    statusLabel->setRect(Rect(0, 0, 860u, 40u));
    statusLabel->setBackgroundColor(Color(52, 152, 219));
    statusLabel->setTextColor(colors::White);
    statusLabel->setPadding(10);
    root->addChild(statusLabel);
    
    // Section 1: Color Variants
    auto section1 = createFlexColumn(10, 15);
    section1->setLayoutWeight(0.0f);
    section1->setBackgroundColor(colors::White);
    section1->setBorder(Color(220, 220, 220), 1.0f);
    
    auto s1Title = std::make_shared<Label>(L"Color Variants");
    s1Title->setFontBold(true);
    s1Title->setFontSize(16.0f);
    section1->addChild(s1Title);
    
    auto colorRow = createFlexRow(10, 0);
    colorRow->setLayoutWeight(0.0f);
    
    // Primary Button
    auto btnPrimary = std::make_shared<Button>(L"Primary");
    btnPrimary->setLayoutWeight(1.0f);
    btnPrimary->setNormalColor(Color(52, 152, 219));
    btnPrimary->setHoverColor(Color(41, 128, 185));
    btnPrimary->setOnClick([statusLabel]() {
        statusLabel->setText(L"✓ Primary button clicked");
    });
    colorRow->addChild(btnPrimary);
    
    // Success Button
    auto btnSuccess = std::make_shared<Button>(L"Success");
    btnSuccess->setLayoutWeight(1.0f);
    btnSuccess->setNormalColor(Color(46, 204, 113));
    btnSuccess->setHoverColor(Color(39, 174, 96));
    btnSuccess->setOnClick([statusLabel]() {
        statusLabel->setText(L"✓ Success button clicked");
    });
    colorRow->addChild(btnSuccess);
    
    // Warning Button
    auto btnWarning = std::make_shared<Button>(L"Warning");
    btnWarning->setLayoutWeight(1.0f);
    btnWarning->setNormalColor(Color(241, 196, 15));
    btnWarning->setHoverColor(Color(243, 156, 18));
    btnWarning->setOnClick([statusLabel]() {
        statusLabel->setText(L"⚠️ Warning button clicked");
    });
    colorRow->addChild(btnWarning);
    
    // Danger Button
    auto btnDanger = std::make_shared<Button>(L"Danger");
    btnDanger->setLayoutWeight(1.0f);
    btnDanger->setNormalColor(Color(231, 76, 60));
    btnDanger->setHoverColor(Color(192, 57, 43));
    btnDanger->setOnClick([statusLabel]() {
        statusLabel->setText(L"❌ Danger button clicked");
    });
    colorRow->addChild(btnDanger);
    
    section1->addChild(colorRow);
    root->addChild(section1);
    
    // Section 2: Size Variants
    auto section2 = createFlexColumn(10, 15);
    section2->setLayoutWeight(0.0f);
    section2->setBackgroundColor(colors::White);
    section2->setBorder(Color(220, 220, 220), 1.0f);
    
    auto s2Title = std::make_shared<Label>(L"Size Variants");
    s2Title->setFontBold(true);
    s2Title->setFontSize(16.0f);
    section2->addChild(s2Title);
    
    auto sizeRow = createFlexRow(10, 0);
    sizeRow->setLayoutWeight(0.0f);
    
    // Small Button
    auto btnSmall = std::make_shared<Button>(L"Small");
    btnSmall->setLayoutWeight(0.0f);
    btnSmall->setRect(Rect(0, 0, 120u, 35u));
    btnSmall->setFontSize(12.0f);
    btnSmall->setNormalColor(Color(149, 165, 166));
    btnSmall->setOnClick([statusLabel]() {
        statusLabel->setText(L"Small button (35px height)");
    });
    sizeRow->addChild(btnSmall);
    
    // Medium Button
    auto btnMedium = std::make_shared<Button>(L"Medium");
    btnMedium->setLayoutWeight(0.0f);
    btnMedium->setRect(Rect(0, 0, 150u, 50u));
    btnMedium->setFontSize(14.0f);
    btnMedium->setNormalColor(Color(149, 165, 166));
    btnMedium->setOnClick([statusLabel]() {
        statusLabel->setText(L"Medium button (50px height)");
    });
    sizeRow->addChild(btnMedium);
    
    // Large Button
    auto btnLarge = std::make_shared<Button>(L"Large");
    btnLarge->setLayoutWeight(0.0f);
    btnLarge->setRect(Rect(0, 0, 180u, 65u));
    btnLarge->setFontSize(16.0f);
    btnLarge->setNormalColor(Color(149, 165, 166));
    btnLarge->setOnClick([statusLabel]() {
        statusLabel->setText(L"Large button (65px height)");
    });
    sizeRow->addChild(btnLarge);
    
    section2->addChild(sizeRow);
    root->addChild(section2);
    
    // Section 3: Special States
    auto section3 = createFlexColumn(10, 15);
    section3->setLayoutWeight(0.0f);
    section3->setBackgroundColor(colors::White);
    section3->setBorder(Color(220, 220, 220), 1.0f);
    
    auto s3Title = std::make_shared<Label>(L"Special States");
    s3Title->setFontBold(true);
    s3Title->setFontSize(16.0f);
    section3->addChild(s3Title);
    
    auto stateRow = createFlexRow(10, 0);
    stateRow->setLayoutWeight(0.0f);
    
    // Disabled Button
    auto btnDisabled = std::make_shared<Button>(L"Disabled");
    btnDisabled->setLayoutWeight(1.0f);
    btnDisabled->setEnabled(false);
    stateRow->addChild(btnDisabled);
    
    // Loading Button (simulated with text)
    auto btnLoading = std::make_shared<Button>(L"⏳ Loading...");
    btnLoading->setLayoutWeight(1.0f);
    btnLoading->setNormalColor(Color(52, 152, 219));
    btnLoading->setEnabled(false);
    stateRow->addChild(btnLoading);
    
    // Toggle Button
    auto btnToggle = std::make_shared<Button>(L"Toggle: OFF");
    btnToggle->setLayoutWeight(1.0f);
    btnToggle->setNormalColor(Color(149, 165, 166));
    
    // Toggle state tracking
    static bool toggleState = false;
    btnToggle->setOnClick([btnToggle, statusLabel]() {
        toggleState = !toggleState;
        if (toggleState) {
            btnToggle->setText(L"Toggle: ON");
            btnToggle->setNormalColor(Color(46, 204, 113));
            statusLabel->setText(L"✓ Toggle enabled");
        } else {
            btnToggle->setText(L"Toggle: OFF");
            btnToggle->setNormalColor(Color(149, 165, 166));
            statusLabel->setText(L"✗ Toggle disabled");
        }
    });
    stateRow->addChild(btnToggle);
    
    section3->addChild(stateRow);
    root->addChild(section3);
    
    // Section 4: Interactive Demo
    auto section4 = createFlexColumn(10, 15);
    section4->setLayoutWeight(0.0f);
    section4->setBackgroundColor(colors::White);
    section4->setBorder(Color(220, 220, 220), 1.0f);
    
    auto s4Title = std::make_shared<Label>(L"Interactive Counter");
    s4Title->setFontBold(true);
    s4Title->setFontSize(16.0f);
    section4->addChild(s4Title);
    
    auto counterLabel = std::make_shared<Label>(L"Count: 0");
    counterLabel->setFontSize(32.0f);
    counterLabel->setFontBold(true);
    counterLabel->setAlignment(Label::Alignment::Center);
    counterLabel->setBackgroundColor(Color(236, 240, 241));
    counterLabel->setPadding(20);
    section4->addChild(counterLabel);
    
    auto counterRow = createFlexRow(10, 0);
    counterRow->setLayoutWeight(0.0f);
    
    static int counter = 0;
    
    // Decrement Button
    auto btnDecrement = std::make_shared<Button>(L"➖ Decrement");
    btnDecrement->setLayoutWeight(1.0f);
    btnDecrement->setNormalColor(Color(231, 76, 60));
    btnDecrement->setOnClick([counterLabel, statusLabel]() {
        counter--;
        counterLabel->setText(std::format(L"Count: {}", counter));
        statusLabel->setText(std::format(L"Decremented to {}", counter));
    });
    counterRow->addChild(btnDecrement);
    
    // Reset Button
    auto btnReset = std::make_shared<Button>(L"🔄 Reset");
    btnReset->setLayoutWeight(1.0f);
    btnReset->setNormalColor(Color(149, 165, 166));
    btnReset->setOnClick([counterLabel, statusLabel]() {
        counter = 0;
        counterLabel->setText(L"Count: 0");
        statusLabel->setText(L"Counter reset to 0");
    });
    counterRow->addChild(btnReset);
    
    // Increment Button
    auto btnIncrement = std::make_shared<Button>(L"➕ Increment");
    btnIncrement->setLayoutWeight(1.0f);
    btnIncrement->setNormalColor(Color(46, 204, 113));
    btnIncrement->setOnClick([counterLabel, statusLabel]() {
        counter++;
        counterLabel->setText(std::format(L"Count: {}", counter));
        statusLabel->setText(std::format(L"Incremented to {}", counter));
    });
    counterRow->addChild(btnIncrement);
    
    section4->addChild(counterRow);
    root->addChild(section4);
    
    window->setRootWidget(root);
    window->show();
    
    std::println("✓ Button Showcase loaded");
    std::println("  - 4 color variants (Primary, Success, Warning, Danger)");
    std::println("  - 3 size variants (Small, Medium, Large)");
    std::println("  - Special states (Disabled, Loading, Toggle)");
    std::println("  - Interactive counter demo");
    
    app.run();
    return 0;
}