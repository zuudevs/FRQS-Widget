// examples/widgets_demo.cpp - Safer Widget Demo
#include "frqs-widget.hpp"
#include "widget/container.hpp"
#include "widget/label.hpp"
#include "widget/button.hpp"
#include "widget/text_input.hpp"
#include "widget/slider.hpp"
#include <print>
#include <format>

using namespace frqs;
using namespace frqs::widget;

// ============================================================================
// DEMO APPLICATION
// ============================================================================

class WidgetDemo {
private:
    std::shared_ptr<Label> statusLabel_;
    std::shared_ptr<Label> sliderValueLabel_;
    std::shared_ptr<TextInput> textInput_;

public:
    void run() {
        try {
            std::println("=== FRQS-Widget: Comprehensive Widget Demo ===\n");
            
            auto& app = Application::instance();
            app.initialize();
            
            // Create main window
            WindowParams params;
            params.title = L"FRQS-Widget Demo - All Widgets";
            params.size = widget::Size(800u, 600u);
            params.position = widget::Point(100, 100);
            
            auto window = app.createWindow(params);
            
            // Build UI
            buildUI(window);
            
            window->show();
            
            std::println("✓ Demo window created");
            std::println("Try out all the widgets!");
            std::println("Close the window to exit.\n");
            
            app.run();
            
            std::println("\nDemo ended successfully.");
            
        } catch (const std::exception& e) {
            std::println(stderr, "ERROR: {}", e.what());
        }
    }

private:
    void buildUI(std::shared_ptr<core::Window> window) {
        // Root container with vertical layout
        auto root = createVStack(10, 15);
        root->setRect(window->getClientRect());
        root->setBackgroundColor(Color(240, 240, 245));
        
        // ====================================================================
        // HEADER
        // ====================================================================
        auto header = std::make_shared<Label>(L"FRQS-Widget Demo");
        header->setRect(Rect(0, 0, 800u, 50u));
        header->setBackgroundColor(Color(52, 152, 219));
        header->setTextColor(colors::White);
        header->setFontSize(24.0f);
        header->setFontBold(true);
        header->setAlignment(Label::Alignment::Center);
        header->setVerticalAlignment(Label::VerticalAlignment::Middle);
        root->addChild(header);
        
        // ====================================================================
        // BUTTONS SECTION
        // ====================================================================
        auto buttonSection = createVStack(8, 0);
        buttonSection->setRect(Rect(0, 0, 800u, 120u));
        buttonSection->setBackgroundColor(colors::White);
        buttonSection->setPadding(15);
        buttonSection->setBorder(Color(200, 200, 200), 1.0f);
        
        auto buttonLabel = std::make_shared<Label>(L"Buttons:");
        buttonLabel->setFontBold(true);
        buttonSection->addChild(buttonLabel);
        
        // Button row
        auto buttonRow = createHStack(10, 0);
        buttonRow->setBackgroundColor(colors::Transparent);
        
        auto primaryBtn = std::make_shared<Button>(L"Primary");
        primaryBtn->setNormalColor(Color(52, 152, 219));
        primaryBtn->setHoverColor(Color(41, 128, 185));
        primaryBtn->setOnClick([this]() {
            // ✅ FIX: Use safe callback
            safeUpdateStatus(L"Primary button clicked!");
            std::println("Primary button clicked!");
        });
        buttonRow->addChild(primaryBtn);
        
        auto successBtn = std::make_shared<Button>(L"Success");
        successBtn->setNormalColor(Color(46, 204, 113));
        successBtn->setHoverColor(Color(39, 174, 96));
        successBtn->setOnClick([this]() {
            safeUpdateStatus(L"Success button clicked!");
            std::println("Success button clicked!");
        });
        buttonRow->addChild(successBtn);
        
        auto dangerBtn = std::make_shared<Button>(L"Danger");
        dangerBtn->setNormalColor(Color(231, 76, 60));
        dangerBtn->setHoverColor(Color(192, 57, 43));
        dangerBtn->setOnClick([this]() {
            safeUpdateStatus(L"Danger button clicked!");
            std::println("Danger button clicked!");
        });
        buttonRow->addChild(dangerBtn);
        
        auto disabledBtn = std::make_shared<Button>(L"Disabled");
        disabledBtn->setEnabled(false);
        buttonRow->addChild(disabledBtn);
        
        buttonSection->addChild(buttonRow);
        root->addChild(buttonSection);
        
        // ====================================================================
        // TEXT INPUT SECTION
        // ====================================================================
        auto inputSection = createVStack(8, 0);
        inputSection->setRect(Rect(0, 0, 800u, 100u));
        inputSection->setBackgroundColor(colors::White);
        inputSection->setPadding(15);
        inputSection->setBorder(Color(200, 200, 200), 1.0f);
        
        auto inputLabel = std::make_shared<Label>(L"Text Input:");
        inputLabel->setFontBold(true);
        inputSection->addChild(inputLabel);
        
        textInput_ = std::make_shared<TextInput>();
        textInput_->setPlaceholder(L"Type something here...");
        textInput_->setRect(Rect(0, 0, 770u, 40u));
        
        // ✅ FIX: Safer callbacks - no recursive invalidation
        textInput_->setOnTextChanged([this](const std::wstring& text) {
            try {
                // Just log, don't update UI during text change
                if (text.length() % 10 == 0) {  // Only log every 10 chars
                    std::println("Text length: {}", text.length());
                }
            } catch (...) {
                // Swallow exceptions in callback
            }
        });
        
        textInput_->setOnEnter([this](const std::wstring& text) {
            try {
                safeUpdateStatus(std::format(L"Enter pressed: {}", text));
                std::println("Enter pressed with text: {}", 
                            std::string(text.begin(), text.end()));
            } catch (...) {
                // Swallow exceptions
            }
        });
        
        inputSection->addChild(textInput_);
        root->addChild(inputSection);
        
        // ====================================================================
        // SLIDER SECTION
        // ====================================================================
        auto sliderSection = createVStack(8, 0);
        sliderSection->setRect(Rect(0, 0, 800u, 120u));
        sliderSection->setBackgroundColor(colors::White);
        sliderSection->setPadding(15);
        sliderSection->setBorder(Color(200, 200, 200), 1.0f);
        
        auto sliderLabel = std::make_shared<Label>(L"Horizontal Slider:");
        sliderLabel->setFontBold(true);
        sliderSection->addChild(sliderLabel);
        
        sliderValueLabel_ = std::make_shared<Label>(L"Value: 50");
        sliderValueLabel_->setTextColor(Color(52, 152, 219));
        sliderSection->addChild(sliderValueLabel_);
        
        auto slider = std::make_shared<Slider>(Slider::Orientation::Horizontal);
        slider->setRect(Rect(0, 0, 770u, 40u));
        slider->setRange(0, 100);
        slider->setValue(50);
        slider->setStep(1);
        slider->setOnValueChanged([this](double value) {
            updateSliderValue(value);
        });
        sliderSection->addChild(slider);
        
        root->addChild(sliderSection);
        
        // ====================================================================
        // LABEL SHOWCASE SECTION
        // ====================================================================
        auto labelSection = createVStack(8, 0);
        labelSection->setRect(Rect(0, 0, 800u, 150u));
        labelSection->setBackgroundColor(colors::White);
        labelSection->setPadding(15);
        labelSection->setBorder(Color(200, 200, 200), 1.0f);
        
        auto labelTitle = std::make_shared<Label>(L"Label Styles:");
        labelTitle->setFontBold(true);
        labelSection->addChild(labelTitle);
        
        // Different label alignments
        auto leftLabel = std::make_shared<Label>(L"Left aligned text");
        leftLabel->setAlignment(Label::Alignment::Left);
        leftLabel->setBackgroundColor(Color(236, 240, 241));
        leftLabel->setPadding(8);
        labelSection->addChild(leftLabel);
        
        auto centerLabel = std::make_shared<Label>(L"Center aligned text");
        centerLabel->setAlignment(Label::Alignment::Center);
        centerLabel->setBackgroundColor(Color(236, 240, 241));
        centerLabel->setPadding(8);
        labelSection->addChild(centerLabel);
        
        auto rightLabel = std::make_shared<Label>(L"Right aligned text");
        rightLabel->setAlignment(Label::Alignment::Right);
        rightLabel->setBackgroundColor(Color(236, 240, 241));
        rightLabel->setPadding(8);
        labelSection->addChild(rightLabel);
        
        root->addChild(labelSection);
        
        // ====================================================================
        // STATUS BAR
        // ====================================================================
        statusLabel_ = std::make_shared<Label>(L"Ready. Try clicking buttons or typing!");
        statusLabel_->setRect(Rect(0, 0, 800u, 40u));
        statusLabel_->setBackgroundColor(Color(52, 73, 94));
        statusLabel_->setTextColor(colors::White);
        statusLabel_->setPadding(10);
        statusLabel_->setVerticalAlignment(Label::VerticalAlignment::Middle);
        root->addChild(statusLabel_);
        
        // Set root widget
        window->setRootWidget(root);
    }
    
    // ✅ FIX: Safe update methods with exception handling
    void safeUpdateStatus(const std::wstring& message) {
        try {
            if (statusLabel_) {
                statusLabel_->setText(message);
            }
        } catch (const std::exception& e) {
            std::println(stderr, "Error updating status: {}", e.what());
        }
    }
    
    void updateSliderValue(double value) {
        try {
            if (sliderValueLabel_) {
                sliderValueLabel_->setText(
                    std::format(L"Value: {:.0f}", value)
                );
            }
            
            std::println("Slider value: {:.1f}", value);
        } catch (const std::exception& e) {
            std::println(stderr, "Error updating slider: {}", e.what());
        }
    }
};

// ============================================================================
// MAIN
// ============================================================================

int main() {
    WidgetDemo demo;
    demo.run();
    return 0;
}