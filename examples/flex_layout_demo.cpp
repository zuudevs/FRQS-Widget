/**
 * @file flex_layout_demo.cpp
 * @brief FlexLayout demonstration
 */

#include "frqs-widget.hpp"
#include "widget/container.hpp"
#include "widget/label.hpp"
#include <print>

using namespace frqs;

class ColoredBox : public widget::Widget {
private:
    std::wstring label_;
    
public:
    ColoredBox(const widget::Color& color, const std::wstring& label)
        : label_(label) {
        setBackgroundColor(color);
    }
    
    void render(widget::Renderer& renderer) override {
        Widget::render(renderer);
        
        auto rect = getRect();
        
        // Draw border
        renderer.drawRect(rect, widget::colors::Black, 2.0f);
        
        // Draw label
        if (auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer)) {
            extRenderer->drawTextEx(
                label_, rect, widget::colors::White,
                render::FontStyle{},
                render::TextAlign::Center,
                render::VerticalAlign::Middle
            );
        }
    }
};

int main() {
    try {
        std::println("=== FlexLayout Demo ===\n");
        
        auto& app = Application::instance();
        app.initialize();
        
        WindowParams params;
        params.title = L"FlexLayout Demo";
        params.size = widget::Size(800u, 600u);
        auto window = app.createWindow(params);
        
        // Main container
        auto mainContainer = widget::createVStack(10, 10);
        mainContainer->setBackgroundColor(widget::Color(240, 240, 245));
        mainContainer->setRect(window->getClientRect());
        
        // Title
        auto title = std::make_shared<widget::Label>(L"Flexbox Layout Examples");
        title->setFontSize(20.0f);
        title->setFontBold(true);
        title->setAlignment(widget::Label::Alignment::Center);
        title->setRect(widget::Rect(0, 0, 800u, 50u));
        mainContainer->addChild(title);
        
        // Example 1: Equal distribution
        auto example1 = widget::createFlexRow(10, 10);
        example1->setRect(widget::Rect(0, 0, 780u, 100u));
        example1->setBackgroundColor(widget::colors::White);
        example1->setBorder(widget::Color(200, 200, 200), 1.0f);
        
        for (int i = 1; i <= 3; i++) {
            auto box = std::make_shared<ColoredBox>(
                widget::Color(52, 152, 219),
                std::format(L"Box {} (weight: 1)", i)
            );
            box->setLayoutWeight(1.0f);
            example1->addChild(box);
        }
        mainContainer->addChild(example1);
        
        // Example 2: Weighted distribution
        auto example2 = widget::createFlexRow(10, 10);
        example2->setRect(widget::Rect(0, 0, 780u, 100u));
        example2->setBackgroundColor(widget::colors::White);
        example2->setBorder(widget::Color(200, 200, 200), 1.0f);
        
        auto box1 = std::make_shared<ColoredBox>(
            widget::Color(46, 204, 113), L"Weight: 1"
        );
        box1->setLayoutWeight(1.0f);
        example2->addChild(box1);
        
        auto box2 = std::make_shared<ColoredBox>(
            widget::Color(155, 89, 182), L"Weight: 2"
        );
        box2->setLayoutWeight(2.0f);
        example2->addChild(box2);
        
        auto box3 = std::make_shared<ColoredBox>(
            widget::Color(231, 76, 60), L"Weight: 1"
        );
        box3->setLayoutWeight(1.0f);
        example2->addChild(box3);
        
        mainContainer->addChild(example2);
        
        // Example 3: Fixed + Flex
        auto example3 = widget::createFlexRow(10, 10);
        example3->setRect(widget::Rect(0, 0, 780u, 100u));
        example3->setBackgroundColor(widget::colors::White);
        example3->setBorder(widget::Color(200, 200, 200), 1.0f);
        
        auto fixed = std::make_shared<ColoredBox>(
            widget::Color(241, 196, 15), L"Fixed: 150px"
        );
        fixed->setLayoutWeight(0.0f);
        fixed->setRect(widget::Rect(0, 0, 150u, 80u));
        example3->addChild(fixed);
        
        auto flex = std::make_shared<ColoredBox>(
            widget::Color(26, 188, 156), L"Flex: Fill"
        );
        flex->setLayoutWeight(1.0f);
        example3->addChild(flex);
        
        mainContainer->addChild(example3);
        
        // Example 4: Vertical flex
        auto example4 = widget::createFlexColumn(10, 10);
        example4->setRect(widget::Rect(0, 0, 780u, 200u));
        example4->setBackgroundColor(widget::colors::White);
        example4->setBorder(widget::Color(200, 200, 200), 1.0f);
        
        for (int i = 1; i <= 3; i++) {
            auto box = std::make_shared<ColoredBox>(
                widget::Color(230, 126, 34),
                std::format(L"Row {}", i)
            );
            box->setLayoutWeight(1.0f);
            example4->addChild(box);
        }
        mainContainer->addChild(example4);
        
        window->setRootWidget(mainContainer);
        window->show();
        
        std::println("✓ FlexLayout demo ready");
        std::println("  Example 1: Equal distribution");
        std::println("  Example 2: Weighted distribution (1:2:1)");
        std::println("  Example 3: Fixed + Flexible");
        std::println("  Example 4: Vertical flex");
        
        app.run();
        return 0;
        
    } catch (const std::exception& e) {
        std::println(stderr, "ERROR: {}", e.what());
        return 1;
    }
}