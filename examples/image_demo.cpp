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