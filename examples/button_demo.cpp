/**
 * @file button_demo.cpp
 * @brief Button widget demonstration
 */

#include "frqs-widget.hpp"
#include "widget/button.hpp"
#include "widget/label.hpp"
#include "widget/container.hpp"
#include <print>

using namespace frqs;

int main() {
    try {
        std::println("=== Button Demo ===\n");
        
        auto& app = Application::instance();
        app.initialize();
        
        WindowParams params;
        params.title = L"Button Demo";
        params.size = widget::Size(600u, 400u);
        auto window = app.createWindow(params);
        
        // Create container with vertical layout
        auto container = widget::createVStack(20, 30);
        container->setBackgroundColor(widget::Color(250, 250, 250));
        container->setRect(window->getClientRect());
        
        // Title label
        auto titleLabel = std::make_shared<widget::Label>(L"Button Widget Examples");
        titleLabel->setFontSize(20.0f);
        titleLabel->setFontBold(true);
        titleLabel->setTextColor(widget::Color(44, 62, 80));
        titleLabel->setAlignment(widget::Label::Alignment::Center);
        titleLabel->setRect(widget::Rect(0, 0, 600u, 50u));
        container->addChild(titleLabel);
        
        // Click counter label
        auto counterLabel = std::make_shared<widget::Label>(L"Clicks: 0");
        counterLabel->setFontSize(16.0f);
        counterLabel->setAlignment(widget::Label::Alignment::Center);
        counterLabel->setRect(widget::Rect(0, 0, 600u, 40u));
        container->addChild(counterLabel);
        
        int clickCount = 0;
        
        // Primary button
        auto primaryBtn = std::make_shared<widget::Button>(L"Primary Button");
        primaryBtn->setRect(widget::Rect(150, 0, 300u, 45u));
        primaryBtn->setNormalColor(widget::Color(52, 152, 219));
        primaryBtn->setHoverColor(widget::Color(41, 128, 185));
        primaryBtn->setPressedColor(widget::Color(21, 101, 192));
        primaryBtn->setOnClick([counterLabel, &clickCount]() {
            clickCount++;
            counterLabel->setText(std::format(L"Clicks: {}", clickCount));
            std::println("Primary button clicked! (Count: {})", clickCount);
        });
        container->addChild(primaryBtn);
        
        // Success button
        auto successBtn = std::make_shared<widget::Button>(L"Success Button");
        successBtn->setRect(widget::Rect(150, 0, 300u, 45u));
        successBtn->setNormalColor(widget::Color(46, 204, 113));
        successBtn->setHoverColor(widget::Color(39, 174, 96));
        successBtn->setPressedColor(widget::Color(34, 153, 84));
        successBtn->setOnClick([counterLabel, &clickCount]() {
            clickCount++;
            counterLabel->setText(std::format(L"Clicks: {}", clickCount));
            std::println("Success button clicked! (Count: {})", clickCount);
        });
        container->addChild(successBtn);
        
        // Warning button
        auto warningBtn = std::make_shared<widget::Button>(L"Warning Button");
        warningBtn->setRect(widget::Rect(150, 0, 300u, 45u));
        warningBtn->setNormalColor(widget::Color(241, 196, 15));
        warningBtn->setHoverColor(widget::Color(243, 156, 18));
        warningBtn->setPressedColor(widget::Color(212, 132, 8));
        warningBtn->setOnClick([counterLabel, &clickCount]() {
            clickCount++;
            counterLabel->setText(std::format(L"Clicks: {}", clickCount));
            std::println("Warning button clicked! (Count: {})", clickCount);
        });
        container->addChild(warningBtn);
        
        // Danger button
        auto dangerBtn = std::make_shared<widget::Button>(L"Danger Button");
        dangerBtn->setRect(widget::Rect(150, 0, 300u, 45u));
        dangerBtn->setNormalColor(widget::Color(231, 76, 60));
        dangerBtn->setHoverColor(widget::Color(192, 57, 43));
        dangerBtn->setPressedColor(widget::Color(169, 50, 38));
        dangerBtn->setOnClick([counterLabel, &clickCount]() {
            clickCount++;
            counterLabel->setText(std::format(L"Clicks: {}", clickCount));
            std::println("Danger button clicked! (Count: {})", clickCount);
        });
        container->addChild(dangerBtn);
        
        // Disabled button
        auto disabledBtn = std::make_shared<widget::Button>(L"Disabled Button");
        disabledBtn->setRect(widget::Rect(150, 0, 300u, 45u));
        disabledBtn->setEnabled(false);
        container->addChild(disabledBtn);
        
        window->setRootWidget(container);
        window->show();
        
        std::println("✓ Button demo ready");
        std::println("  Try clicking different buttons!");
        
        app.run();
        return 0;
        
    } catch (const std::exception& e) {
        std::println(stderr, "ERROR: {}", e.what());
        return 1;
    }
}