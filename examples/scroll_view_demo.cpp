// examples/scrollview_verification.cpp - Verify ScrollView Fix
#include "frqs-widget.hpp"
#include "widget/container.hpp"
#include "widget/button.hpp"
#include "widget/scroll_view.hpp"
#include "widget/label.hpp"
#include <print>
#include <format>

using namespace frqs;
using namespace frqs::widget;

// ============================================================================
// VERIFICATION TEST: ScrollView Coordinate Transform
// ============================================================================

int main() {
    try {
        std::println("=== ScrollView Bug Fix Verification ===\n");
        
        auto& app = Application::instance();
        app.initialize();
        
        // Create window
        WindowParams params;
        params.title = L"ScrollView Fix Verification";
        params.size = Size(700u, 500u);
        params.position = Point(100, 100);
        
        auto window = app.createWindow(params);
        
        // ====================================================================
        // TEST 1: Basic ScrollView with Long Content
        // ====================================================================
        
        std::println("Creating ScrollView with 100 buttons...");
        
        auto scrollView = std::make_shared<ScrollView>();
        scrollView->setRect(window->getClientRect());
        scrollView->setBackgroundColor(Color(245, 245, 245));
        
        // Create long content (100 buttons = 5000px+ height)
        auto contentContainer = createFlexColumn(5, 10);
        contentContainer->setBackgroundColor(colors::White);
        
        for (int i = 0; i < 100; ++i) {
            auto button = std::make_shared<Button>(
                std::format(L"Button #{:03d} - Click to Test", i + 1)
            );
            
            // Set fixed height
            button->setLayoutWeight(0.0f);
            button->setRect(Rect(0, 0, 660u, 45u));
            
            // Color coding for easy visual verification
            if (i < 25) {
                button->setNormalColor(Color(231, 76, 60));   // Red (top)
            } else if (i < 50) {
                button->setNormalColor(Color(52, 152, 219));  // Blue (upper-middle)
            } else if (i < 75) {
                button->setNormalColor(Color(46, 204, 113));  // Green (lower-middle)
            } else {
                button->setNormalColor(Color(155, 89, 182));  // Purple (bottom)
            }
            
            // Add click handler to verify coordinate transform
            button->setOnClick([i]() {
                std::println("✓ Button #{} clicked - Coordinates CORRECT!", i + 1);
            });
            
            contentContainer->addChild(button);
        }
        
        // Calculate content size
        uint32_t contentHeight = 100 * 45 + 99 * 5 + 20;
        contentContainer->setRect(Rect(0, 0, 660u, contentHeight));
        
        scrollView->setContent(contentContainer);
        window->setRootWidget(scrollView);
        window->show();
        
        std::println("✓ ScrollView created successfully");
        std::println("");
        std::println("VERIFICATION INSTRUCTIONS:");
        std::println("════════════════════════════════════════════════");
        std::println("1. Use mouse wheel to scroll down");
        std::println("   → Content should move smoothly");
        std::println("   → Scrollbar thumb should move proportionally");
        std::println("");
        std::println("2. Drag scrollbar thumb to bottom");
        std::println("   → Purple buttons (Button #76-100) should be visible");
        std::println("   → Content should shift correctly");
        std::println("");
        std::println("3. Click on visible buttons");
        std::println("   → Correct button number should be logged");
        std::println("   → No coordinate mismatch");
        std::println("");
        std::println("4. Color coding (for easy verification):");
        std::println("   • Red buttons (#01-25)  = Top section");
        std::println("   • Blue buttons (#26-50) = Upper-middle");
        std::println("   • Green buttons (#51-75) = Lower-middle");
        std::println("   • Purple buttons (#76-100) = Bottom section");
        std::println("");
        std::println("EXPECTED BEHAVIOR:");
        std::println("════════════════════════════════════════════════");
        std::println("✓ Scrolling moves content visually");
        std::println("✓ Scrollbar position matches content offset");
        std::println("✓ Clicking buttons triggers correct callbacks");
        std::println("✓ No coordinate drift or accumulation");
        std::println("✓ Smooth animation without flickering");
        std::println("");
        std::println("BUG SYMPTOMS (if not fixed):");
        std::println("════════════════════════════════════════════════");
        std::println("✗ Scrollbar moves but content stays at (0,0)");
        std::println("✗ Clicking on visible button triggers wrong callback");
        std::println("✗ Coordinate mismatch between visual and logical position");
        std::println("");
        std::println("Close the window to exit.\n");
        
        // Run event loop
        app.run();
        
        std::println("\nVerification completed.");
        std::println("If scrolling worked correctly, the fix is successful!");
        
        return 0;
        
    } catch (const std::exception& e) {
        std::println(stderr, "ERROR: {}", e.what());
        return 1;
    }
}

// ============================================================================
// NESTED SCROLLVIEW TEST (Advanced Verification)
// ============================================================================

void createNestedScrollViewTest() {
    auto& app = Application::instance();
    
    WindowParams params;
    params.title = L"Nested ScrollView Test";
    params.size = Size(800u, 600u);
    
    auto window = app.createWindow(params);
    
    // Outer ScrollView (vertical)
    auto outerScroll = std::make_shared<ScrollView>();
    outerScroll->setRect(window->getClientRect());
    outerScroll->setBackgroundColor(Color(240, 240, 245));
    
    auto outerContent = createFlexColumn(20, 10);
    
    // Header
    auto header = std::make_shared<Label>(L"Outer ScrollView - Scroll Vertically");
    header->setBackgroundColor(Color(52, 73, 94));
    header->setTextColor(colors::White);
    header->setFontSize(18.0f);
    header->setFontBold(true);
    header->setLayoutWeight(0.0f);
    header->setRect(Rect(0, 0, 760u, 60u));
    outerContent->addChild(header);
    
    // Inner ScrollView (horizontal) - embedded in outer
    auto innerScroll = std::make_shared<ScrollView>();
    innerScroll->setBackgroundColor(colors::White);
    innerScroll->setLayoutWeight(0.0f);
    innerScroll->setRect(Rect(0, 0, 760u, 300u));
    innerScroll->setVerticalScrollEnabled(false);  // Only horizontal scroll
    
    // Wide content for horizontal scrolling
    auto innerContent = createFlexRow(10, 10);
    innerContent->setBackgroundColor(Color(236, 240, 241));
    
    for (int i = 0; i < 20; ++i) {
        auto card = std::make_shared<Label>(std::format(L"Card\n#{}", i+1));
        card->setBackgroundColor(Color(52, 152, 219));
        card->setTextColor(colors::White);
        card->setAlignment(Label::Alignment::Center);
        card->setVerticalAlignment(Label::VerticalAlignment::Middle);
        card->setLayoutWeight(0.0f);
        card->setRect(Rect(0, 0, 200u, 280u));
        innerContent->addChild(card);
    }
    
    innerContent->setRect(Rect(0, 0, 4200u, 280u));  // Wide!
    innerScroll->setContent(innerContent);
    
    outerContent->addChild(innerScroll);
    
    // More content below for vertical scrolling
    for (int i = 0; i < 30; ++i) {
        auto item = std::make_shared<Label>(
            std::format(L"Outer Item #{} - Scroll vertically to see", i+1)
        );
        item->setBackgroundColor(Color(149, 165, 166));
        item->setTextColor(colors::White);
        item->setPadding(15);
        item->setLayoutWeight(0.0f);
        item->setRect(Rect(0, 0, 760u, 50u));
        outerContent->addChild(item);
    }
    
    uint32_t totalHeight = 60 + 300 + 20 + (30 * 50) + (29 * 20) + 20;
    outerContent->setRect(Rect(0, 0, 760u, totalHeight));
    
    outerScroll->setContent(outerContent);
    window->setRootWidget(outerScroll);
    window->show();
    
    std::println("\n=== Nested ScrollView Test Created ===");
    std::println("This tests transform stack with nested scroll layers:");
    std::println("  • Outer ScrollView: Vertical scrolling");
    std::println("  • Inner ScrollView: Horizontal scrolling");
    std::println("");
    std::println("Verification:");
    std::println("  1. Scroll vertically in outer area");
    std::println("  2. Scroll horizontally in inner card list");
    std::println("  3. Both should work independently");
    std::println("  4. No coordinate corruption between layers\n");
}