#include "frqs-widget.hpp"
#include "widget/container.hpp"
#include "widget/label.hpp"
#include "widget/checkbox.hpp"
#include "widget/image.hpp"
#include "widget/button.hpp"
#include <print>

using namespace frqs;
using namespace frqs::widget;

int main() {
    try {
        std::println("=== Image & Checkbox Demo (Fixed) ===");
        
        auto& app = Application::instance();
        app.initialize();
        
        // Setup Window
        WindowParams params;
        params.title = L"Phase 3 Demo - Layout Fix";
        params.size = Size(850u, 650u);
        params.position = Point(100, 100);
        
        auto window = app.createWindow(params);
        
        // Root Layout
        auto root = createFlexRow(0, 0); 
        root->setBackgroundColor(Color(245, 245, 245));
        auto sidebar = createFlexColumn(15, 15);
        sidebar->setLayoutWeight(0.0f); // Fixed
        sidebar->setRect(Rect(0, 0, 280u, 650u)); 
        sidebar->setBackgroundColor(Color(52, 73, 94));
        
        // Title Sidebar
        auto title = std::make_shared<Label>(L"Settings");
        title->setTextColor(colors::White);
        title->setFontSize(18.0f);
        title->setFontBold(true);
        title->setLayoutWeight(0.0f);
        title->setRect(Rect(0, 0, 250u, 40u)); // Height 40
        sidebar->addChild(title);

        auto cbSection = createFlexColumn(10, 10);
        cbSection->setLayoutWeight(0.0f); // Fixed Height Container
        cbSection->setRect(Rect(0, 0, 250u, 140u)); 
        cbSection->setBackgroundColor(Color(44, 62, 80));
        cbSection->setBorder(Color(34, 52, 70), 1.0f);

        auto cb1 = std::make_shared<CheckBox>(L"Show Image Border");
        cb1->setTextColor(colors::White);
        cb1->setRect(Rect(0, 0, 230u, 30u));
        cb1->setChecked(true); // Default ON
        cbSection->addChild(cb1);

        auto cb2 = std::make_shared<CheckBox>(L"Allow Stretch");
        cb2->setTextColor(colors::White);
        cb2->setRect(Rect(0, 0, 230u, 30u));
        cbSection->addChild(cb2);

        sidebar->addChild(cbSection);
        root->addChild(sidebar);

        // Content area
        auto content = createFlexColumn(20, 20);
        content->setLayoutWeight(1.0f); // Ambil sisa width
        content->setBackgroundColor(colors::White);

        // Header
        auto header = std::make_shared<Label>(L"Image Preview");
        header->setFontSize(24.0f);
        header->setLayoutWeight(0.0f);
        header->setRect(Rect(0, 0, 500u, 50u));
        content->addChild(header);

        // Image Frame
        auto imageFrame = std::make_shared<Container>();
        imageFrame->setLayoutWeight(1.0f); // Ambil sisa height ke bawah
        imageFrame->setBackgroundColor(Color(230, 230, 230));
        imageFrame->setBorder(Color(231, 76, 60), 4.0f); 
        imageFrame->setLayout(std::make_unique<FlexLayout>(FlexLayout::Direction::Column, 0, 0));
        auto image = std::make_shared<Image>(L"test_image.png");
        image->setLayoutWeight(1.0f);
        image->setScaleMode(Image::ScaleMode::Fit);
        
        imageFrame->addChild(image);
        content->addChild(imageFrame);
        cb1->setOnChanged([imageFrame](bool checked) {
            if (checked) {
                imageFrame->setBorder(Color(231, 76, 60), 4.0f);
            } else {
                imageFrame->setBorder(colors::Transparent, 0.0f);
            }
        });

        // Toggle Scale Mode
        cb2->setOnChanged([image](bool checked) {
            if (checked) image->setScaleMode(Image::ScaleMode::Stretch);
            else image->setScaleMode(Image::ScaleMode::Fit);
        });

        root->addChild(content);
        window->setRootWidget(root);
        root->setRect(window->getClientRect());
        std::static_pointer_cast<Container>(root)->applyLayout();

        window->show();
        
        std::println("✓ Demo created. Layout forced manually.");
        app.run();
        
    } catch (const std::exception& e) {
        std::println(stderr, "ERROR: {}", e.what());
    }
    return 0;
}