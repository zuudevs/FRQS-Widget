/**
 * @file layout_demo.cpp
 * @brief Different layout types demonstration
 */

#include "frqs-widget.hpp"
#include "widget/container.hpp"
#include "widget/label.hpp"
#include <print>

using namespace frqs;

class ColorBox : public widget::Widget {
private:
    std::wstring text_;
    
public:
    explicit ColorBox(const widget::Color& color, const std::wstring& text = L"")
        : text_(text) {
        setBackgroundColor(color);
    }
    
    void render(widget::Renderer& renderer) override {
        Widget::render(renderer);
        
        auto rect = getRect();
        renderer.drawRect(rect, widget::colors::Black, 1.0f);
        
        if (!text_.empty()) {
            if (auto* extRenderer = dynamic_cast<render::IExtendedRenderer*>(&renderer)) {
                extRenderer->drawTextEx(
                    text_, rect, widget::colors::White,
                    render::FontStyle{L"Segoe UI", 12.0f, true},
                    render::TextAlign::Center,
                    render::VerticalAlign::Middle
                );
            }
        }
    }
};

int main() {
    try {
        std::println("=== Layout Demo ===\n");
        
        auto& app = Application::instance();
        app.initialize();
        
        WindowParams params;
        params.title = L"Layout Examples";
        params.size = widget::Size(800u, 600u);
        auto window = app.createWindow(params);
        
        // Main vertical container
        auto mainContainer = widget::createVStack(10, 10);
        mainContainer->setBackgroundColor(widget::Color(240, 240, 245));
        mainContainer->setRect(window->getClientRect());
        
        // Title
        auto title = std::make_shared<widget::Label>(L"Layout System Examples");
        title->setFontSize(20.0f);
        title->setFontBold(true);
        title->setAlignment(widget::Label::Alignment::Center);
        title->setRect(widget::Rect(0, 0, 800u, 50u));
        mainContainer->addChild(title);
        
        // Grid Layout Example
        auto gridLabel = std::make_shared<widget::Label>(L"Grid Layout (3x3):");
        gridLabel->setFontBold(true);
        gridLabel->setRect(widget::Rect(0, 0, 780u, 25u));
        mainContainer->addChild(gridLabel);
        
        auto gridContainer = widget::createGrid(3, 3, 5, 5);
        gridContainer->setRect(widget::Rect(0, 0, 780u, 180u));
        gridContainer->setBackgroundColor(widget::colors::White);
        
        std::vector<widget::Color> colors = {
            widget::Color(52, 152, 219),
            widget::Color(46, 204, 113),
            widget::Color(155, 89, 182),
            widget::Color(241, 196, 15),
            widget::Color(231, 76, 60),
            widget::Color(26, 188, 156),
            widget::Color(230, 126, 34),
            widget::Color(189, 195, 199),
            widget::Color(52, 73, 94)
        };
        
        for (int i = 0; i < 9; i++) {
            auto box = std::make_shared<ColorBox>(
                colors[i],
                std::format(L"Cell {}", i + 1)
            );
            gridContainer->addChild(box);
        }
        mainContainer->addChild(gridContainer);
        
        // Stack Layout Example
        auto stackLabel = std::make_shared<widget::Label>(L"Horizontal Stack:");
        stackLabel->setFontBold(true);
        stackLabel->setRect(widget::Rect(0, 0, 780u, 25u));
        mainContainer->addChild(stackLabel);
        
        auto hStack = widget::createHStack(10, 10);
        hStack->setRect(widget::Rect(0, 0, 780u, 100u));
        hStack->setBackgroundColor(widget::colors::White);
        
        for (int i = 0; i < 4; i++) {
            auto box = std::make_shared<ColorBox>(
                colors[i],
                std::format(L"Box {}", i + 1)
            );
            box->setRect(widget::Rect(0, 0, 180u, 80u));
            hStack->addChild(box);
        }
        mainContainer->addChild(hStack);
        
        // Flex Layout Example
        auto flexLabel = std::make_shared<widget::Label>(L"Flex Layout (Weighted):");
        flexLabel->setFontBold(true);
        flexLabel->setRect(widget::Rect(0, 0, 780u, 25u));
        mainContainer->addChild(flexLabel);
        
        auto flexContainer = widget::createFlexRow(10, 10);
        flexContainer->setRect(widget::Rect(0, 0, 780u, 100u));
        flexContainer->setBackgroundColor(widget::colors::White);
        
        auto flexBox1 = std::make_shared<ColorBox>(colors[0], L"1x");
        flexBox1->setLayoutWeight(1.0f);
        flexContainer->addChild(flexBox1);
        
        auto flexBox2 = std::make_shared<ColorBox>(colors[1], L"2x");
        flexBox2->setLayoutWeight(2.0f);
        flexContainer->addChild(flexBox2);
        
        auto flexBox3 = std::make_shared<ColorBox>(colors[2], L"1x");
        flexBox3->setLayoutWeight(1.0f);
        flexContainer->addChild(flexBox3);
        
        mainContainer->addChild(flexContainer);
        
        window->setRootWidget(mainContainer);
        window->show();
        
        std::println("✓ Layout demo ready");
        std::println("  Grid Layout: 3x3 cells");
        std::println("  Stack Layout: Horizontal arrangement");
        std::println("  Flex Layout: Weighted distribution (1:2:1)");
        
        app.run();
        return 0;
        
    } catch (const std::exception& e) {
        std::println(stderr, "ERROR: {}", e.what());
        return 1;
    }
}