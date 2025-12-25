/**
 * @file scrollview_demo.cpp
 * @brief ScrollView widget demonstration
 */

#include "frqs-widget.hpp"
#include "widget/scroll_view.hpp"
#include "widget/container.hpp"
#include "widget/label.hpp"
#include <print>

using namespace frqs;

int main() {
    try {
        std::println("=== ScrollView Demo ===\n");
        
        auto& app = Application::instance();
        app.initialize();
        
        WindowParams params;
        params.title = L"ScrollView Demo";
        params.size = widget::Size(600u, 400u);
        auto window = app.createWindow(params);
        
        // Create ScrollView
        auto scrollView = std::make_shared<widget::ScrollView>();
        scrollView->setRect(window->getClientRect());
        scrollView->setBackgroundColor(widget::Color(240, 240, 245));
        
        // Create content (larger than viewport)
        auto content = widget::createVStack(10, 20);
        content->setRect(widget::Rect(0, 0, 800u, 1200u));
        content->setBackgroundColor(widget::colors::White);
        
        // Add title
        auto title = std::make_shared<widget::Label>(L"Scrollable Content");
        title->setFontSize(24.0f);
        title->setFontBold(true);
        title->setAlignment(widget::Label::Alignment::Center);
        title->setRect(widget::Rect(0, 0, 800u, 60u));
        content->addChild(title);
        
        // Add many items
        for (int i = 1; i <= 30; i++) {
            auto item = std::make_shared<widget::Widget>();
            item->setRect(widget::Rect(0, 0, 760u, 35u));
            
            // Alternate colors
            if (i % 2 == 0) {
                item->setBackgroundColor(widget::Color(250, 250, 250));
            } else {
                item->setBackgroundColor(widget::Color(240, 240, 245));
            }
            
            auto label = std::make_shared<widget::Label>(
                std::format(L"Item {} - This is a scrollable item", i)
            );
            label->setRect(widget::Rect(20, 0, 740u, 35u));
            label->setVerticalAlignment(widget::Label::VerticalAlignment::Middle);
            item->addChild(label);
            
            content->addChild(item);
        }
        
        scrollView->setContent(content);
        window->setRootWidget(scrollView);
        window->show();
        
        std::println("✓ ScrollView demo ready");
        std::println("  Content size: 800x1200");
        std::println("  Viewport size: 600x400");
        std::println("  Use mouse wheel or drag scrollbar to scroll");
        
        app.run();
        return 0;
        
    } catch (const std::exception& e) {
        std::println(stderr, "ERROR: {}", e.what());
        return 1;
    }
}