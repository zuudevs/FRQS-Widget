// examples/button_demo.cpp - Comprehensive Button Demo

/**
 * @file button_demo.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Demonstrates various button styles, states, and configurations.
 * @version 0.2
 * @date 2025-12-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "frqs-widget.hpp"
#include "widget/container.hpp"
#include "widget/button.hpp"
#include "widget/label.hpp"
#include <print>
#include <format>

using namespace frqs;
using namespace frqs::widget;

class ButtonDemo {
private:
    std::shared_ptr<Label> statusLabel_;

public:
    void run() {
        std::println("=== FRQS Button Showcase ===");
        
        auto& app = Application::instance();
        app.initialize();
        
        WindowParams params;
        params.title = L"Button Showcase - All Styles & States";
        params.size = Size(1000u, 800u);
        params.position = Point(100, 100);
        
        auto window = app.createWindow(params);
        
        buildUI(window);
        
        window->show();
        app.run();
    }

private:
    void buildUI(std::shared_ptr<core::Window> window) {
        // Root container
        auto root = createFlexColumn(20, 20);
        root->setRect(window->getClientRect());
        root->setBackgroundColor(Color(240, 242, 245));

        // Header
        auto header = std::make_shared<Label>(L"🔘 FRQS Button Styles");
        header->setFontSize(28.0f);
        header->setFontBold(true);
        header->setTextColor(Color(44, 62, 80));
        header->setLayoutWeight(0.0f);
        header->setRect(Rect(0, 0, 960u, 60u));
        header->setAlignment(Label::Alignment::Center);
        root->addChild(header);

        // 1. Semantic Colors
        addSection(root, L"Semantic Colors", [this](std::shared_ptr<Container> container) {
            auto row = createFlexRow(15, 0);
            row->setLayoutWeight(0.0f);
            row->setRect(Rect(0, 0, 900u, 50u));

            row->addChild(createButton(L"Primary", Color(52, 152, 219)));    // Blue
            row->addChild(createButton(L"Success", Color(46, 204, 113)));    // Green
            row->addChild(createButton(L"Warning", Color(241, 196, 15)));    // Yellow
            row->addChild(createButton(L"Danger", Color(231, 76, 60)));      // Red
            row->addChild(createButton(L"Info", Color(155, 89, 182)));       // Purple

            container->addChild(row);
        });

        // 2. States
        addSection(root, L"Button States", [this](std::shared_ptr<Container> container) {
            auto row = createFlexRow(15, 0);
            row->setLayoutWeight(0.0f);
            row->setRect(Rect(0, 0, 900u, 50u));

            auto btnNormal = createButton(L"Normal State", Color(52, 73, 94));
            row->addChild(btnNormal);

            auto btnDisabled = createButton(L"Disabled State", Color(52, 73, 94));
            btnDisabled->setEnabled(false);
            row->addChild(btnDisabled);

            container->addChild(row);
        });

        // 3. Sizes
        addSection(root, L"Button Sizes", [this](std::shared_ptr<Container> container) {
            auto row = createFlexRow(15, 0);
            row->setLayoutWeight(0.0f);
            row->setRect(Rect(0, 0, 900u, 70u));
            
            auto btnSmall = createButton(L"Small", Color(52, 152, 219));
            btnSmall->setRect(Rect(0, 0, 100u, 30u));
            btnSmall->setFontSize(12.0f);
            row->addChild(btnSmall);

            auto btnMedium = createButton(L"Medium", Color(52, 152, 219));
            btnMedium->setRect(Rect(0, 0, 140u, 45u)); // Standard
            row->addChild(btnMedium);

            auto btnLarge = createButton(L"Large Button", Color(52, 152, 219));
            btnLarge->setRect(Rect(0, 0, 200u, 60u));
            btnLarge->setFontSize(20.0f);
            row->addChild(btnLarge);

            container->addChild(row);
        });

        // 4. Shapes & Borders
        addSection(root, L"Shapes & Borders", [this](std::shared_ptr<Container> container) {
            auto row = createFlexRow(15, 0);
            row->setLayoutWeight(0.0f);
            row->setRect(Rect(0, 0, 900u, 50u));

            // Rounded (Default usually, but explicit here)
            auto btnRounded = createButton(L"Rounded (10px)", Color(230, 126, 34));
            btnRounded->setBorderRadius(10.0f);
            row->addChild(btnRounded);

            // Pill Shape
            auto btnPill = createButton(L"Pill Shape", Color(230, 126, 34));
            btnPill->setBorderRadius(22.5f); // Half of height (45/2)
            row->addChild(btnPill);

            // Square
            auto btnSquare = createButton(L"Square (0px)", Color(230, 126, 34));
            btnSquare->setBorderRadius(0.0f);
            row->addChild(btnSquare);

            // Outlined
            auto btnOutlined = createButton(L"Outlined", colors::White);
            btnOutlined->setTextColor(Color(230, 126, 34));
            btnOutlined->setBorder(Color(230, 126, 34), 2.0f);
            btnOutlined->setHoverColor(Color(250, 240, 230)); // Very light orange
            row->addChild(btnOutlined);

            container->addChild(row);
        });

        // Status Bar
        statusLabel_ = std::make_shared<Label>(L"Ready. Click any button.");
        statusLabel_->setLayoutWeight(0.0f);
        statusLabel_->setRect(Rect(0, 0, 960u, 40u));
        statusLabel_->setBackgroundColor(Color(52, 73, 94));
        statusLabel_->setTextColor(colors::White);
        statusLabel_->setPadding(10);
        statusLabel_->setAlignment(Label::Alignment::Center);
        root->addChild(statusLabel_);

        window->setRootWidget(root);
    }

    // Helper to create a section container
    void addSection(std::shared_ptr<Container> root, const std::wstring& title, 
                   std::function<void(std::shared_ptr<Container>)> contentBuilder) {
        auto section = createFlexColumn(15, 20);
        section->setLayoutWeight(0.0f);
        section->setBackgroundColor(colors::White);
        section->setBorder(Color(220, 220, 220), 1.0f);
        section->setRect(Rect(0, 0, 960u, 150u)); // Approximate height, flex will handle content

        auto label = std::make_shared<Label>(title);
        label->setFontSize(16.0f);
        label->setFontBold(true);
        label->setTextColor(Color(100, 100, 100));
        label->setLayoutWeight(0.0f);
        label->setRect(Rect(0, 0, 200u, 30u));
        section->addChild(label);

        contentBuilder(section);
        root->addChild(section);
    }

    // Helper to create a standard button
    std::shared_ptr<Button> createButton(const std::wstring& text, const Color& color) {
        auto btn = std::make_shared<Button>(text);
        btn->setNormalColor(color);
        
        // Simple darken for hover
        int r = std::max(0, (int)color.r - 20);
        int g = std::max(0, (int)color.g - 20);
        int b = std::max(0, (int)color.b - 20);
        btn->setHoverColor(Color((uint8_t)r, (uint8_t)g, (uint8_t)b));
        
        btn->setLayoutWeight(0.0f);
        btn->setRect(Rect(0, 0, 160u, 45u)); // Default size
        
        btn->setOnClick([this, text]() {
            if (statusLabel_) {
                statusLabel_->setText(std::format(L"Action: Clicked '{}'", text));
            }
            std::println("Button clicked: {}", std::string(text.begin(), text.end()));
        });
        
        return btn;
    }
};

int main() {
    ButtonDemo demo;
    demo.run();
    return 0;
}