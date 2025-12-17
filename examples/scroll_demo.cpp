// examples/scroll_demo.cpp - ScrollView Demonstration
#include "frqs-widget.hpp"
#include "widget/container.hpp"
#include "widget/label.hpp"
#include "widget/button.hpp"
#include "widget/scroll_view.hpp"
#include <print>
#include <format>

using namespace frqs;
using namespace frqs::widget;

// ============================================================================
// DEMO: Long List of Interactive Buttons
// ============================================================================

int main() {
    try {
        std::println("=== FRQS-Widget: ScrollView Demo ===\n");
        
        auto& app = Application::instance();
        app.initialize();
        
        // Create main window
        WindowParams params;
        params.title = L"ScrollView Demo - Interactive Button List";
        params.size = Size(600u, 400u);  // Small viewport
        params.position = Point(100, 100);
        
        auto window = app.createWindow(params);
        
        // ====================================================================
        // CREATE SCROLL VIEW
        // ====================================================================
        
        auto scrollView = std::make_shared<ScrollView>();
        scrollView->setRect(window->getClientRect());
        scrollView->setBackgroundColor(Color(245, 245, 245));
        
        // ====================================================================
        // CREATE CONTENT (LONG LIST)
        // ====================================================================
        
        auto contentContainer = createFlexColumn(8, 10);
        contentContainer->setBackgroundColor(colors::White);
        
        // Create 50 interactive buttons (will overflow viewport)
        for (int i = 0; i < 50; ++i) {
            auto button = std::make_shared<Button>(
                std::format(L"Button #{:02d} - Click Me!", i + 1)
            );
            
            // Set fixed height
            button->setLayoutWeight(0.0f);
            button->setRect(Rect(0, 0, 560u, 50u));
            
            // Alternating colors
            if (i % 3 == 0) {
                button->setNormalColor(Color(52, 152, 219));   // Blue
            } else if (i % 3 == 1) {
                button->setNormalColor(Color(46, 204, 113));   // Green
            } else {
                button->setNormalColor(Color(155, 89, 182));   // Purple
            }
            
            // Add click handler
            button->setOnClick([i]() {
                std::println("✓ Button #{} clicked!", i + 1);
            });
            
            contentContainer->addChild(button);
        }
        
        // ====================================================================
        // SETUP HIERARCHY
        // ====================================================================
        
        // Calculate content size (50 buttons × 50px + 49 gaps × 8px + 20px padding)
        uint32_t contentHeight = 50 * 50 + 49 * 8 + 20;
        contentContainer->setRect(Rect(0, 0, 560u, contentHeight));
        
        scrollView->setContent(contentContainer);
        window->setRootWidget(scrollView);
        window->show();
        
        std::println("✓ ScrollView demo created");
        std::println("  - Viewport: 600×400px");
        std::println("  - Content: 560×{}px (scrollable!)", contentHeight);
        std::println("  - 50 interactive buttons");
        std::println("");
        std::println("Try:");
        std::println("  • Mouse wheel to scroll");
        std::println("  • Drag scrollbar thumb");
        std::println("  • Click buttons to test interaction");
        std::println("\nClose the window to exit.\n");
        
        // Run event loop
        app.run();
        
        std::println("\nDemo ended successfully.");
        return 0;
        
    } catch (const std::exception& e) {
        std::println(stderr, "ERROR: {}", e.what());
        return 1;
    }
}