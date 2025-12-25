/**
 * @file image_and_checkbox_demo.cpp
 * @brief Combined Image and CheckBox demonstration
 */

#include "frqs-widget.hpp"
#include "widget/image.hpp"
#include "widget/checkbox.hpp"
#include "widget/container.hpp"
#include "widget/label.hpp"
#include <print>

using namespace frqs;

int main() {
    try {
        std::println("=== Image & CheckBox Demo ===\n");
        
        auto& app = Application::instance();
        app.initialize();
        
        WindowParams params;
        params.title = L"Image & CheckBox Demo";
        params.size = widget::Size(700u, 500u);
        auto window = app.createWindow(params);
        
        // Main container
        auto mainContainer = widget::createVStack(20, 20);
        mainContainer->setBackgroundColor(widget::Color(250, 250, 250));
        mainContainer->setRect(window->getClientRect());
        
        // Title
        auto title = std::make_shared<widget::Label>(L"Image Display Options");
        title->setFontSize(20.0f);
        title->setFontBold(true);
        title->setAlignment(widget::Label::Alignment::Center);
        title->setRect(widget::Rect(0, 0, 700u, 50u));
        mainContainer->addChild(title);
        
        // Image container
        auto imageContainer = std::make_shared<widget::Widget>();
        imageContainer->setRect(widget::Rect(0, 0, 660u, 300u));
        imageContainer->setBackgroundColor(widget::Color(240, 240, 245));
        
        // Image widget (provide path to your image)
        auto image = std::make_shared<widget::Image>(L"path/to/your/image.png");
        image->setRect(widget::Rect(10, 10, 640u, 280u));
        image->setScaleMode(widget::Image::ScaleMode::Fit);
        imageContainer->addChild(image);
        
        mainContainer->addChild(imageContainer);
        
        // Options container
        auto optionsContainer = widget::createVStack(10, 20);
        optionsContainer->setRect(widget::Rect(0, 0, 660u, 100u));
        
        // Scale mode options
        auto fitCheckbox = std::make_shared<widget::CheckBox>(L"Fit (Preserve Aspect Ratio)");
        fitCheckbox->setChecked(true);
        fitCheckbox->setRect(widget::Rect(0, 0, 300u, 30u));
        fitCheckbox->setOnChanged([image, fitCheckbox](bool checked) {
            if (checked) {
                image->setScaleMode(widget::Image::ScaleMode::Fit);
                std::println("Scale mode: Fit");
            }
        });
        optionsContainer->addChild(fitCheckbox);
        
        auto fillCheckbox = std::make_shared<widget::CheckBox>(L"Fill (Crop to Fill)");
        fillCheckbox->setRect(widget::Rect(0, 0, 300u, 30u));
        fillCheckbox->setOnChanged([image, fitCheckbox](bool checked) {
            if (checked) {
                image->setScaleMode(widget::Image::ScaleMode::Fill);
                fitCheckbox->setChecked(false);
                std::println("Scale mode: Fill");
            }
        });
        optionsContainer->addChild(fillCheckbox);
        
        auto stretchCheckbox = std::make_shared<widget::CheckBox>(L"Stretch (Ignore Aspect Ratio)");
        stretchCheckbox->setRect(widget::Rect(0, 0, 300u, 30u));
        stretchCheckbox->setOnChanged([image, fitCheckbox](bool checked) {
            if (checked) {
                image->setScaleMode(widget::Image::ScaleMode::Stretch);
                fitCheckbox->setChecked(false);
                std::println("Scale mode: Stretch");
            }
        });
        optionsContainer->addChild(stretchCheckbox);
        
        mainContainer->addChild(optionsContainer);
        
        window->setRootWidget(mainContainer);
        window->show();
        
        std::println("✓ Image & CheckBox demo ready");
        std::println("  Note: Update image path in the code to see image");
        std::println("  Try different scale modes using checkboxes");
        
        app.run();
        return 0;
        
    } catch (const std::exception& e) {
        std::println(stderr, "ERROR: {}", e.what());
        return 1;
    }
}