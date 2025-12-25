/**
 * @file checkbox_demo.cpp
 * @brief CheckBox widget demonstration
 */

#include "frqs-widget.hpp"
#include "widget/checkbox.hpp"
#include "widget/label.hpp"
#include "widget/container.hpp"
#include <print>

using namespace frqs;

int main() {
    try {
        std::println("=== CheckBox Demo ===\n");
        
        auto& app = Application::instance();
        app.initialize();
        
        WindowParams params;
        params.title = L"CheckBox Demo";
        params.size = widget::Size(500u, 400u);
        auto window = app.createWindow(params);
        
        // Create container
        auto container = widget::createVStack(15, 30);
        container->setBackgroundColor(widget::Color(250, 250, 250));
        container->setRect(window->getClientRect());
        
        // Title
        auto title = std::make_shared<widget::Label>(L"CheckBox Examples");
        title->setFontSize(20.0f);
        title->setFontBold(true);
        title->setAlignment(widget::Label::Alignment::Center);
        title->setRect(widget::Rect(0, 0, 500u, 50u));
        container->addChild(title);
        
        // Status label
        auto statusLabel = std::make_shared<widget::Label>(L"Selected: None");
        statusLabel->setAlignment(widget::Label::Alignment::Center);
        statusLabel->setRect(widget::Rect(0, 0, 500u, 30u));
        container->addChild(statusLabel);
        
        std::vector<std::wstring> selectedItems;
        
        auto updateStatus = [statusLabel, &selectedItems]() {
            if (selectedItems.empty()) {
                statusLabel->setText(L"Selected: None");
            } else {
                std::wstring text = L"Selected: ";
                for (size_t i = 0; i < selectedItems.size(); ++i) {
                    if (i > 0) text += L", ";
                    text += selectedItems[i];
                }
                statusLabel->setText(text);
            }
        };
        
        // Create checkboxes
        std::vector<std::wstring> options = {
            L"Enable Notifications",
            L"Auto-save Files",
            L"Dark Mode",
            L"Show Line Numbers",
            L"Word Wrap"
        };
        
        for (const auto& option : options) {
            auto checkbox = std::make_shared<widget::CheckBox>(option);
            checkbox->setRect(widget::Rect(50, 0, 400u, 35u));
            checkbox->setOnChanged([option, &selectedItems, updateStatus](bool checked) {
                if (checked) {
                    selectedItems.push_back(option);
                    std::println("✓ Checked: {}", 
                        std::string(option.begin(), option.end()));
                } else {
                    auto it = std::find(selectedItems.begin(), 
                                      selectedItems.end(), option);
                    if (it != selectedItems.end()) {
                        selectedItems.erase(it);
                    }
                    std::println("✗ Unchecked: {}", 
                        std::string(option.begin(), option.end()));
                }
                updateStatus();
            });
            container->addChild(checkbox);
        }
        
        window->setRootWidget(container);
        window->show();
        
        std::println("✓ CheckBox demo ready");
        
        app.run();
        return 0;
        
    } catch (const std::exception& e) {
        std::println(stderr, "ERROR: {}", e.what());
        return 1;
    }
}