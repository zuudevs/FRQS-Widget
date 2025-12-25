/**
 * @file widget_demo.cpp
 * @brief Various widget types demonstration
 */

#include "frqs-widget.hpp"
#include "widget/button.hpp"
#include "widget/label.hpp"
#include "widget/text_input.hpp"
#include "widget/slider.hpp"
#include "widget/container.hpp"
#include <print>

using namespace frqs;

int main() {
    try {
        std::println("=== Widget Demo ===\n");
        
        auto& app = Application::instance();
        app.initialize();
        
        WindowParams params;
        params.title = L"Widget Gallery";
        params.size = widget::Size(600u, 500u);
        auto window = app.createWindow(params);
        
        // Main container
        auto container = widget::createVStack(15, 20);
        container->setBackgroundColor(widget::Color(250, 250, 250));
        container->setRect(window->getClientRect());
        
        // Title
        auto title = std::make_shared<widget::Label>(L"Widget Gallery");
        title->setFontSize(24.0f);
        title->setFontBold(true);
        title->setAlignment(widget::Label::Alignment::Center);
        title->setRect(widget::Rect(0, 0, 600u, 50u));
        container->addChild(title);
        
        // Label section
        auto labelHeader = std::make_shared<widget::Label>(L"Label Widget:");
        labelHeader->setFontBold(true);
        labelHeader->setRect(widget::Rect(0, 0, 560u, 25u));
        container->addChild(labelHeader);
        
        auto label = std::make_shared<widget::Label>(L"This is a label with some text");
        label->setRect(widget::Rect(20, 0, 540u, 25u));
        label->setTextColor(widget::Color(52, 73, 94));
        container->addChild(label);
        
        // TextInput section
        auto inputHeader = std::make_shared<widget::Label>(L"TextInput Widget:");
        inputHeader->setFontBold(true);
        inputHeader->setRect(widget::Rect(0, 0, 560u, 25u));
        container->addChild(inputHeader);
        
        auto textInput = std::make_shared<widget::TextInput>();
        textInput->setRect(widget::Rect(20, 0, 540u, 40u));
        textInput->setPlaceholder(L"Type something here...");
        textInput->setOnTextChanged([](const std::wstring& text) {
            std::println("Text changed: {}", 
                std::string(text.begin(), text.end()));
        });
        container->addChild(textInput);
        
        // Slider section
        auto sliderHeader = std::make_shared<widget::Label>(L"Slider Widget:");
        sliderHeader->setFontBold(true);
        sliderHeader->setRect(widget::Rect(0, 0, 560u, 25u));
        container->addChild(sliderHeader);
        
        auto sliderValueLabel = std::make_shared<widget::Label>(L"Value: 50");
        sliderValueLabel->setRect(widget::Rect(20, 0, 540u, 25u));
        container->addChild(sliderValueLabel);
        
        auto slider = std::make_shared<widget::Slider>(
            widget::Slider::Orientation::Horizontal
        );
        slider->setRect(widget::Rect(20, 0, 540u, 40u));
        slider->setRange(0.0, 100.0);
        slider->setValue(50.0);
        slider->setOnValueChanged([sliderValueLabel](double value) {
            sliderValueLabel->setText(std::format(L"Value: {:.0f}", value));
            std::println("Slider value: {:.1f}", value);
        });
        container->addChild(slider);
        
        // Button section
        auto buttonHeader = std::make_shared<widget::Label>(L"Button Widget:");
        buttonHeader->setFontBold(true);
        buttonHeader->setRect(widget::Rect(0, 0, 560u, 25u));
        container->addChild(buttonHeader);
        
        auto button = std::make_shared<widget::Button>(L"Click Me!");
        button->setRect(widget::Rect(200, 0, 200u, 45u));
        button->setOnClick([textInput]() {
            textInput->setText(L"Button was clicked!");
            std::println("Button clicked!");
        });
        container->addChild(button);
        
        window->setRootWidget(container);
        window->show();
        
        std::println("✓ Widget demo ready");
        std::println("  Try interacting with different widgets");
        
        app.run();
        return 0;
        
    } catch (const std::exception& e) {
        std::println(stderr, "ERROR: {}", e.what());
        return 1;
    }
}