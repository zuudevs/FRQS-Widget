// examples/flex_layout_demo.cpp - FlexLayout Usage Example
#include "frqs-widget.hpp"
#include "widget/container.hpp"
#include "widget/label.hpp"
#include <print>

using namespace frqs;
using namespace frqs::widget;

// ============================================================================
// FLEX LAYOUT EXAMPLE: Sidebar (Fixed) + Content (Flex)
// ============================================================================

int main() {
    try {
        std::println("=== FRQS-Widget: FlexLayout Demo ===\n");
        
        auto& app = Application::instance();
        app.initialize();
        
        // Create main window
        WindowParams params;
        params.title = L"FlexLayout Demo - Sidebar + Content";
        params.size = Size(1000u, 600u);
        params.position = Point(100, 100);
        
        auto window = app.createWindow(params);
        
        // ====================================================================
        // ROOT CONTAINER - Horizontal FlexLayout (Row)
        // ====================================================================
        
        auto root = createFlexRow(0, 0);  // No gap, no padding
        root->setRect(window->getClientRect());
        root->setBackgroundColor(Color(240, 240, 245));
        
        // ====================================================================
        // SIDEBAR - Fixed Width (250px)
        // ====================================================================
        
        auto sidebar = std::make_shared<Label>(L"Sidebar (Fixed 250px)");
        sidebar->setBackgroundColor(Color(52, 73, 94));  // Dark blue
        sidebar->setTextColor(colors::White);
        sidebar->setFontSize(16.0f);
        sidebar->setAlignment(Label::Alignment::Center);
        sidebar->setVerticalAlignment(Label::VerticalAlignment::Top);
        sidebar->setPadding(20);
        
        // Set fixed width (weight = 0 means fixed size)
        sidebar->setLayoutWeight(0.0f);
        sidebar->setRect(Rect(0, 0, 250u, 600u));  // Initial size: 250x600
        
        root->addChild(sidebar);
        
        // ====================================================================
        // CONTENT AREA - Flexible (takes remaining space)
        // ====================================================================
        
        auto content = std::make_shared<Container>();
        content->setBackgroundColor(colors::White);
        
        // Set flex weight (weight = 1.0 means "take remaining space")
        content->setLayoutWeight(1.0f);
        
        // Create vertical layout inside content area
        content->setLayout(std::make_unique<FlexLayout>(
            FlexLayout::Direction::Column, 10, 20
        ));
        
        // ====================================================================
        // HEADER - Fixed Height (80px)
        // ====================================================================
        
        auto header = std::make_shared<Label>(L"Header (Fixed 80px)");
        header->setBackgroundColor(Color(52, 152, 219));  // Blue
        header->setTextColor(colors::White);
        header->setFontSize(20.0f);
        header->setFontBold(true);
        header->setAlignment(Label::Alignment::Center);
        header->setVerticalAlignment(Label::VerticalAlignment::Middle);
        
        header->setLayoutWeight(0.0f);  // Fixed size
        header->setRect(Rect(0, 0, 750u, 80u));  // Initial size
        
        content->addChild(header);
        
        // ====================================================================
        // MAIN CONTENT - Flexible (takes remaining space)
        // ====================================================================
        
        auto mainContent = std::make_shared<Label>(
            L"Main Content Area\n(Flex: takes remaining space)\n\nResize the window to see flex layout in action!"
        );
        mainContent->setBackgroundColor(Color(236, 240, 241));
        mainContent->setTextColor(Color(44, 62, 80));
        mainContent->setFontSize(14.0f);
        mainContent->setAlignment(Label::Alignment::Center);
        mainContent->setVerticalAlignment(Label::VerticalAlignment::Middle);
        
        mainContent->setLayoutWeight(1.0f);  // Flex: takes remaining space
        
        content->addChild(mainContent);
        
        // ====================================================================
        // FOOTER - Fixed Height (60px)
        // ====================================================================
        
        auto footer = std::make_shared<Label>(L"Footer (Fixed 60px)");
        footer->setBackgroundColor(Color(149, 165, 166));  // Gray
        footer->setTextColor(colors::White);
        footer->setFontSize(12.0f);
        footer->setAlignment(Label::Alignment::Center);
        footer->setVerticalAlignment(Label::VerticalAlignment::Middle);
        
        footer->setLayoutWeight(0.0f);  // Fixed size
        footer->setRect(Rect(0, 0, 750u, 60u));  // Initial size
        
        content->addChild(footer);
        
        // ====================================================================
        // Add content to root
        // ====================================================================
        
        root->addChild(content);
        
        // Set root widget
        window->setRootWidget(root);
        window->show();
        
        std::println("✓ FlexLayout demo window created");
        std::println("Resize the window to see flex layout in action!");
        std::println("Close the window to exit.\n");
        
        // Run event loop
        app.run();
        
        std::println("\nDemo ended successfully.");
        return 0;
        
    } catch (const std::exception& e) {
        std::println(stderr, "ERROR: {}", e.what());
        return 1;
    }
}

// ============================================================================
// ADVANCED EXAMPLE: Three-Column Layout with Weighted Distribution
// ============================================================================

void createThreeColumnLayout() {
    auto& app = Application::instance();
    
    // Create window
    WindowParams params;
    params.title = L"FlexLayout - Three Columns (1:2:1 ratio)";
    params.size = Size(1200u, 600u);
    
    auto window = app.createWindow(params);
    
    // Root container (horizontal)
    auto root = createFlexRow(10, 10);
    root->setRect(window->getClientRect());
    root->setBackgroundColor(Color(240, 240, 245));
    
    // Left column (weight = 1)
    auto leftCol = std::make_shared<Label>(L"Left\n(weight: 1)");
    leftCol->setBackgroundColor(Color(231, 76, 60));
    leftCol->setTextColor(colors::White);
    leftCol->setAlignment(Label::Alignment::Center);
    leftCol->setVerticalAlignment(Label::VerticalAlignment::Middle);
    leftCol->setLayoutWeight(1.0f);
    
    // Center column (weight = 2) - twice as wide as others
    auto centerCol = std::make_shared<Label>(L"Center\n(weight: 2)\nTwice as wide!");
    centerCol->setBackgroundColor(Color(46, 204, 113));
    centerCol->setTextColor(colors::White);
    centerCol->setAlignment(Label::Alignment::Center);
    centerCol->setVerticalAlignment(Label::VerticalAlignment::Middle);
    centerCol->setLayoutWeight(2.0f);
    
    // Right column (weight = 1)
    auto rightCol = std::make_shared<Label>(L"Right\n(weight: 1)");
    rightCol->setBackgroundColor(Color(52, 152, 219));
    rightCol->setTextColor(colors::White);
    rightCol->setAlignment(Label::Alignment::Center);
    rightCol->setVerticalAlignment(Label::VerticalAlignment::Middle);
    rightCol->setLayoutWeight(1.0f);
    
    root->addChild(leftCol);
    root->addChild(centerCol);
    root->addChild(rightCol);
    
    window->setRootWidget(root);
    window->show();
}

// ============================================================================
// EXAMPLE: Alignment Options
// ============================================================================

void createAlignmentDemo() {
    auto& app = Application::instance();
    
    WindowParams params;
    params.title = L"FlexLayout - Alignment Demo";
    params.size = Size(800u, 600u);
    
    auto window = app.createWindow(params);
    
    // Root container (vertical)
    auto root = createFlexColumn(20, 20);
    root->setRect(window->getClientRect());
    root->setBackgroundColor(Color(240, 240, 245));
    
    // Title (fixed)
    auto title = std::make_shared<Label>(L"Alignment Options");
    title->setBackgroundColor(Color(52, 73, 94));
    title->setTextColor(colors::White);
    title->setFontSize(20.0f);
    title->setFontBold(true);
    title->setLayoutWeight(0.0f);
    title->setRect(Rect(0, 0, 760u, 60u));
    root->addChild(title);
    
    // Demo boxes with different alignments
    auto startBox = std::make_shared<Label>(L"Align: Start");
    startBox->setBackgroundColor(Color(231, 76, 60));
    startBox->setTextColor(colors::White);
    startBox->setLayoutWeight(0.0f);
    startBox->setAlignSelf(LayoutProps::Align::Start);
    startBox->setRect(Rect(0, 0, 200u, 80u));
    root->addChild(startBox);
    
    auto centerBox = std::make_shared<Label>(L"Align: Center");
    centerBox->setBackgroundColor(Color(46, 204, 113));
    centerBox->setTextColor(colors::White);
    centerBox->setLayoutWeight(0.0f);
    centerBox->setAlignSelf(LayoutProps::Align::Center);
    centerBox->setRect(Rect(0, 0, 200u, 80u));
    root->addChild(centerBox);
    
    auto endBox = std::make_shared<Label>(L"Align: End");
    endBox->setBackgroundColor(Color(52, 152, 219));
    endBox->setTextColor(colors::White);
    endBox->setLayoutWeight(0.0f);
    endBox->setAlignSelf(LayoutProps::Align::End);
    endBox->setRect(Rect(0, 0, 200u, 80u));
    root->addChild(endBox);
    
    auto stretchBox = std::make_shared<Label>(L"Align: Stretch (default)");
    stretchBox->setBackgroundColor(Color(149, 165, 166));
    stretchBox->setTextColor(colors::White);
    stretchBox->setLayoutWeight(1.0f);
    stretchBox->setAlignSelf(LayoutProps::Align::Stretch);
    root->addChild(stretchBox);
    
    window->setRootWidget(root);
    window->show();
}