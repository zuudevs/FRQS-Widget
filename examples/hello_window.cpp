// examples/hello_window.cpp - Minimal Hello World Example
#include "frqs-widget.hpp"
#include <print>

using namespace frqs;

// ============================================================================
// MINIMAL EXAMPLE - Just Create and Show a Window
// ============================================================================

int main() {
    try {
        std::println("=== FRQS-Widget: Hello Window ===\n");
        
        // Get application instance
        auto& app = Application::instance();
        
        // Initialize
        app.initialize();
        
        // Create a simple window
        WindowParams params;
        params.title = L"Hello FRQS-Widget!";
        params.size = widget::Size(640u, 480u);
        params.position = widget::Point(200, 150);
        
        auto window = app.createWindow(params);
        
        // Create a simple colored widget
        auto root = std::make_shared<widget::Widget>();
        root->setRect(window->getClientRect());
        root->setBackgroundColor(widget::Color(70, 130, 180));  // Steel Blue
        
        window->setRootWidget(root);
        window->show();
        
        std::println("✓ Window created and shown");
        std::println("Close the window to exit.\n");
        
        // Run event loop
        app.run();
        
        std::println("Goodbye!");
        return 0;
        
    } catch (const std::exception& e) {
        std::println(stderr, "ERROR: {}", e.what());
        return 1;
    }
}