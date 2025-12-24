// examples/layout_showcase.cpp - FlexLayout System Demo

/**
 * @file layout_demo.cpp
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
#include <print>

using namespace frqs;
using namespace frqs::widget;

// Helper to create colored box
auto createBox(const std::wstring& label, const Color& color, float weight = 0.0f) {
    auto box = std::make_shared<Label>(label);
    box->setBackgroundColor(color);
    box->setTextColor(colors::White);
    box->setFontBold(true);
    box->setAlignment(Label::Alignment::Center);
    box->setVerticalAlignment(Label::VerticalAlignment::Middle);
    box->setLayoutWeight(weight);
    if (weight == 0.0f) {
        box->setRect(Rect(0, 0, 100u, 100u));
    }
    return box;
}

int main() {
    std::println("=== FRQS Layout System Showcase ===");
    
    auto& app = Application::instance();
    app.initialize();
    
    WindowParams params;
    params.title = L"Layout System Showcase - FlexLayout Power";
    params.size = Size(1200u, 800u);
    params.position = Point(50, 50);
    
    auto window = app.createWindow(params);
    
    // Root: Horizontal split
    auto root = createFlexRow(0, 0);
    root->setRect(window->getClientRect());
    
    // LEFT PANEL: Examples
    auto leftPanel = createFlexColumn(20, 20);
    leftPanel->setLayoutWeight(1.0f);
    leftPanel->setBackgroundColor(Color(245, 245, 250));
    
    // Title
    auto title = std::make_shared<Label>(L"📐 Layout Examples");
    title->setLayoutWeight(0.0f);
    title->setRect(Rect(0, 0, 500u, 50u));
    title->setFontSize(20.0f);
    title->setFontBold(true);
    title->setBackgroundColor(Color(52, 73, 94));
    title->setTextColor(colors::White);
    title->setAlignment(Label::Alignment::Center);
    leftPanel->addChild(title);
    
    // Example 1: Equal Distribution
    auto ex1Title = std::make_shared<Label>(L"1. Equal Distribution (weight: 1-1-1)");
    ex1Title->setLayoutWeight(0.0f);
    ex1Title->setFontBold(true);
    leftPanel->addChild(ex1Title);
    
    auto ex1 = createFlexRow(10, 0);
    ex1->setLayoutWeight(0.0f);
    ex1->setRect(Rect(0, 0, 500u, 80u));
    ex1->addChild(createBox(L"A", Color(231, 76, 60), 1.0f));
    ex1->addChild(createBox(L"B", Color(52, 152, 219), 1.0f));
    ex1->addChild(createBox(L"C", Color(46, 204, 113), 1.0f));
    leftPanel->addChild(ex1);
    
    // Example 2: Weighted Distribution
    auto ex2Title = std::make_shared<Label>(L"2. Weighted Distribution (weight: 1-2-1)");
    ex2Title->setLayoutWeight(0.0f);
    ex2Title->setFontBold(true);
    leftPanel->addChild(ex2Title);
    
    auto ex2 = createFlexRow(10, 0);
    ex2->setLayoutWeight(0.0f);
    ex2->setRect(Rect(0, 0, 500u, 80u));
    ex2->addChild(createBox(L"Small", Color(155, 89, 182), 1.0f));
    ex2->addChild(createBox(L"Large", Color(52, 152, 219), 2.0f));
    ex2->addChild(createBox(L"Small", Color(155, 89, 182), 1.0f));
    leftPanel->addChild(ex2);
    
    // Example 3: Fixed + Flex
    auto ex3Title = std::make_shared<Label>(L"3. Fixed Sidebar + Flex Content");
    ex3Title->setLayoutWeight(0.0f);
    ex3Title->setFontBold(true);
    leftPanel->addChild(ex3Title);
    
    auto ex3 = createFlexRow(10, 0);
    ex3->setLayoutWeight(0.0f);
    ex3->setRect(Rect(0, 0, 500u, 80u));
    auto sidebar = createBox(L"Fixed\n150px", Color(52, 73, 94), 0.0f);
    sidebar->setRect(Rect(0, 0, 150u, 80u));
    ex3->addChild(sidebar);
    ex3->addChild(createBox(L"Flex Content", Color(52, 152, 219), 1.0f));
    leftPanel->addChild(ex3);
    
    // Example 4: Nested Layout
    auto ex4Title = std::make_shared<Label>(L"4. Nested Layouts (Column in Row)");
    ex4Title->setLayoutWeight(0.0f);
    ex4Title->setFontBold(true);
    leftPanel->addChild(ex4Title);
    
    auto ex4 = createFlexRow(10, 0);
    ex4->setLayoutWeight(0.0f);
    ex4->setRect(Rect(0, 0, 500u, 120u));
    
    auto nested = createFlexColumn(5, 0);
    nested->setLayoutWeight(1.0f);
    nested->addChild(createBox(L"Top", Color(241, 196, 15), 1.0f));
    nested->addChild(createBox(L"Bottom", Color(230, 126, 34), 1.0f));
    
    ex4->addChild(createBox(L"Left", Color(231, 76, 60), 1.0f));
    ex4->addChild(nested);
    ex4->addChild(createBox(L"Right", Color(46, 204, 113), 1.0f));
    leftPanel->addChild(ex4);
    
    // Example 5: Gap Spacing
    auto ex5Title = std::make_shared<Label>(L"5. Gap Spacing (20px gaps)");
    ex5Title->setLayoutWeight(0.0f);
    ex5Title->setFontBold(true);
    leftPanel->addChild(ex5Title);
    
    auto ex5 = createFlexRow(20, 0);
    ex5->setLayoutWeight(0.0f);
    ex5->setRect(Rect(0, 0, 500u, 80u));
    ex5->addChild(createBox(L"1", Color(52, 152, 219), 1.0f));
    ex5->addChild(createBox(L"2", Color(52, 152, 219), 1.0f));
    ex5->addChild(createBox(L"3", Color(52, 152, 219), 1.0f));
    ex5->addChild(createBox(L"4", Color(52, 152, 219), 1.0f));
    leftPanel->addChild(ex5);
    
    root->addChild(leftPanel);
    
    // RIGHT PANEL: Real-World Example
    auto rightPanel = createFlexColumn(0, 0);
    rightPanel->setLayoutWeight(0.0f);
    rightPanel->setRect(Rect(0, 0, 450u, 800u));
    rightPanel->setBackgroundColor(Color(44, 62, 80));
    
    // App Layout Simulation
    auto appTitle = std::make_shared<Label>(L"💼 Real App Layout");
    appTitle->setLayoutWeight(0.0f);
    appTitle->setRect(Rect(0, 0, 450u, 50u));
    appTitle->setFontSize(18.0f);
    appTitle->setFontBold(true);
    appTitle->setBackgroundColor(Color(52, 73, 94));
    appTitle->setTextColor(colors::White);
    appTitle->setAlignment(Label::Alignment::Center);
    rightPanel->addChild(appTitle);
    
    // Header
    auto header = std::make_shared<Label>(L"📋 Dashboard Header");
    header->setLayoutWeight(0.0f);
    header->setRect(Rect(0, 0, 450u, 60u));
    header->setBackgroundColor(Color(41, 128, 185));
    header->setTextColor(colors::White);
    header->setFontBold(true);
    header->setAlignment(Label::Alignment::Center);
    rightPanel->addChild(header);
    
    // Content Area with Sidebar
    auto contentArea = createFlexRow(0, 0);
    contentArea->setLayoutWeight(1.0f);
    
    // Sidebar
    auto sidebarArea = createFlexColumn(5, 10);
    sidebarArea->setLayoutWeight(0.0f);
    sidebarArea->setRect(Rect(0, 0, 120u, 600u));
    sidebarArea->setBackgroundColor(Color(52, 73, 94));
    
    auto navItem1 = std::make_shared<Button>(L"🏠 Home");
    navItem1->setLayoutWeight(0.0f);
    navItem1->setRect(Rect(0, 0, 100u, 40u));
    navItem1->setNormalColor(Color(41, 128, 185));
    sidebarArea->addChild(navItem1);
    
    auto navItem2 = std::make_shared<Button>(L"📊 Stats");
    navItem2->setLayoutWeight(0.0f);
    navItem2->setRect(Rect(0, 0, 100u, 40u));
    navItem2->setNormalColor(Color(52, 73, 94));
    sidebarArea->addChild(navItem2);
    
    auto navItem3 = std::make_shared<Button>(L"⚙️ Settings");
    navItem3->setLayoutWeight(0.0f);
    navItem3->setRect(Rect(0, 0, 100u, 40u));
    navItem3->setNormalColor(Color(52, 73, 94));
    sidebarArea->addChild(navItem3);
    
    contentArea->addChild(sidebarArea);
    
    // Main Content
    auto mainContent = createFlexColumn(10, 15);
    mainContent->setLayoutWeight(1.0f);
    mainContent->setBackgroundColor(Color(236, 240, 241));
    
    // Card 1
    auto card1 = std::make_shared<Label>(L"📈 Analytics Card\n\nTotal Users: 1,234\nActive: 789");
    card1->setLayoutWeight(0.0f);
    card1->setRect(Rect(0, 0, 300u, 100u));
    card1->setBackgroundColor(colors::White);
    card1->setPadding(15);
    card1->setBorder(Color(189, 195, 199), 1.0f);
    mainContent->addChild(card1);
    
    // Card 2
    auto card2 = std::make_shared<Label>(L"💰 Revenue Card\n\nToday: $5,678\nMonth: $123,456");
    card2->setLayoutWeight(0.0f);
    card2->setRect(Rect(0, 0, 300u, 100u));
    card2->setBackgroundColor(colors::White);
    card2->setPadding(15);
    card2->setBorder(Color(189, 195, 199), 1.0f);
    mainContent->addChild(card2);
    
    // Card 3
    auto card3 = std::make_shared<Label>(L"🔔 Notifications\n\n3 new messages\n2 pending tasks");
    card3->setLayoutWeight(0.0f);
    card3->setRect(Rect(0, 0, 300u, 100u));
    card3->setBackgroundColor(colors::White);
    card3->setPadding(15);
    card3->setBorder(Color(189, 195, 199), 1.0f);
    mainContent->addChild(card3);
    
    contentArea->addChild(mainContent);
    rightPanel->addChild(contentArea);
    
    // Footer
    auto footer = std::make_shared<Label>(L"© 2025 FRQS-Widget Framework");
    footer->setLayoutWeight(0.0f);
    footer->setRect(Rect(0, 0, 450u, 40u));
    footer->setBackgroundColor(Color(52, 73, 94));
    footer->setTextColor(colors::White);
    footer->setAlignment(Label::Alignment::Center);
    rightPanel->addChild(footer);
    
    root->addChild(rightPanel);
    
    window->setRootWidget(root);
    window->show();
    
    std::println("✓ Layout Showcase loaded");
    std::println("  - Equal distribution (1-1-1 weights)");
    std::println("  - Weighted distribution (1-2-1 weights)");
    std::println("  - Fixed + Flex layouts");
    std::println("  - Nested layouts");
    std::println("  - Gap spacing");
    std::println("  - Real dashboard example");
    
    app.run();
    return 0;
}