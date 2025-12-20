// examples/scroll_view_demo.cpp
// Final Version: Fixed Padding & Verification

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
// VERIFICATION TEST: ScrollView Fix & Padding Adjustment
// ============================================================================

int main() {
    try {
        std::println("=== ScrollView Verification (Final) ===\n");
        
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
        
        // [FIX UTAMA DI SINI]
        // createFlexColumn(gap, padding)
        // Ubah padding jadi 0 supaya tombol mentok kanan (Full Width)
        // Ini mencegah "dead zone" saat mouse ada di pinggir kanan scrollbar.
        auto contentContainer = createFlexColumn(5, 0); 
        contentContainer->setBackgroundColor(colors::White);
        
        // Lebar container = Lebar Window (700)
        // Scrollbar = 12px (default)
        // Jadi area konten aman sekitar 688px. Kita set 660px biar aman.
        
        for (int i = 0; i < 100; ++i) {
            auto button = std::make_shared<Button>(
                std::format(L"Button #{:03d} - Click to Test", i + 1)
            );
            
            // Set fixed height
            button->setLayoutWeight(0.0f);
            // Lebar tombol 660px (mengisi container)
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
            
            // Add click handler
            button->setOnClick([i]() {
                // Log ini hanya muncul saat klik sukses
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
        std::println("✓ Padding set to 0 (Full Width Buttons)");
        std::println("✓ Hover Debug logs should appear in terminal (from Library)");
        std::println("");
        std::println("Close the window to exit.\n");
        
        // Run event loop
        app.run();
        
        return 0;
        
    } catch (const std::exception& e) {
        std::println(stderr, "ERROR: {}", e.what());
        return 1;
    }
}