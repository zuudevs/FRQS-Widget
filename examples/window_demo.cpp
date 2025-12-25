/**
 * @file window_demo.cpp
 * @brief Basic window creation and management demo
 */

#include "frqs-widget.hpp"
#include <print>

using namespace frqs;

int main() {
    try {
        std::println("=== Window Demo ===\n");
        
        auto& app = Application::instance();
        app.initialize();
        
        // Create main window
        WindowParams params;
        params.title = L"Window Demo - FRQS Widget";
        params.size = widget::Size(800u, 600u);
        params.position = widget::Point(100, 100);
        params.resizable = true;
        params.visible = true;
        
        auto window = app.createWindow(params);
        
        // Create simple content
        auto root = std::make_shared<widget::Widget>();
        root->setBackgroundColor(widget::Color(240, 240, 245));
        root->setRect(window->getClientRect());
        
        window->setRootWidget(root);
        
        std::println("✓ Window created successfully");
        std::println("  Title: {}", "Window Demo - FRQS Widget");
        std::println("  Size: {}x{}", params.size.w, params.size.h);
        std::println("\nClose window to exit.");
        
        app.run();
        
        return 0;
    } catch (const std::exception& e) {
        std::println(stderr, "ERROR: {}", e.what());
        return 1;
    }
}