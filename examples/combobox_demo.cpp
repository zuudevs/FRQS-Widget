/**
 * @file combobox_demo.cpp
 * @brief ComboBox dropdown widget demonstration
 */

#include "frqs-widget.hpp"
#include "widget/combobox.hpp"
#include "widget/list_adapter.hpp"
#include "widget/label.hpp"
#include "widget/container.hpp"
#include <print>
#include <vector>

using namespace frqs;

// Simple string adapter
class StringListAdapter : public widget::IListAdapter {
private:
    std::vector<std::wstring> items_;
    
public:
    explicit StringListAdapter(std::vector<std::wstring> items)
        : items_(std::move(items)) {}
    
    size_t getCount() const override {
        return items_.size();
    }
    
    std::shared_ptr<widget::IWidget> createView(size_t) override {
        auto label = std::make_shared<widget::Label>();
        label->setBackgroundColor(widget::colors::White);
        label->setPadding(10);
        return label;
    }
    
    void updateView(size_t index, widget::IWidget* view) override {
        if (index >= items_.size()) return;
        
        if (auto* label = dynamic_cast<widget::Label*>(view)) {
            label->setText(items_[index]);
        }
    }
};

int main() {
    try {
        std::println("=== ComboBox Demo ===\n");
        
        auto& app = Application::instance();
        app.initialize();
        
        WindowParams params;
        params.title = L"ComboBox Demo";
        params.size = widget::Size(500u, 400u);
        auto window = app.createWindow(params);
        
        // Main container
        auto container = widget::createVStack(20, 30);
        container->setBackgroundColor(widget::Color(250, 250, 250));
        container->setRect(window->getClientRect());
        
        // Title
        auto title = std::make_shared<widget::Label>(L"ComboBox Examples");
        title->setFontSize(20.0f);
        title->setFontBold(true);
        title->setAlignment(widget::Label::Alignment::Center);
        title->setRect(widget::Rect(0, 0, 500u, 50u));
        container->addChild(title);
        
        // Selection label
        auto selectionLabel = std::make_shared<widget::Label>(L"Selected: None");
        selectionLabel->setAlignment(widget::Label::Alignment::Center);
        selectionLabel->setRect(widget::Rect(0, 0, 500u, 30u));
        container->addChild(selectionLabel);
        
        // Country selector
        auto countryLabel = std::make_shared<widget::Label>(L"Select Country:");
        countryLabel->setRect(widget::Rect(50, 0, 400u, 25u));
        container->addChild(countryLabel);
        
        auto countryCombo = std::make_shared<widget::ComboBox>();
        countryCombo->setRect(widget::Rect(50, 0, 400u, 40u));
        
        std::vector<std::wstring> countries = {
            L"Indonesia", L"Singapore", L"Malaysia", L"Thailand",
            L"Philippines", L"Vietnam", L"Japan", L"South Korea"
        };
        
        auto countryAdapter = std::make_shared<StringListAdapter>(countries);
        countryCombo->setAdapter(countryAdapter);
        countryCombo->setOnSelectionChanged([selectionLabel, countries](size_t index) {
            if (index < countries.size()) {
                selectionLabel->setText(std::format(L"Selected: {}", countries[index]));
                std::println("Country selected: {}", 
                    std::string(countries[index].begin(), countries[index].end()));
            }
        });
        container->addChild(countryCombo);
        
        // Programming language selector
        auto langLabel = std::make_shared<widget::Label>(L"Select Language:");
        langLabel->setRect(widget::Rect(50, 0, 400u, 25u));
        container->addChild(langLabel);
        
        auto langCombo = std::make_shared<widget::ComboBox>();
        langCombo->setRect(widget::Rect(50, 0, 400u, 40u));
        
        std::vector<std::wstring> languages = {
            L"C++", L"Python", L"JavaScript", L"Java",
            L"C#", L"Go", L"Rust", L"TypeScript"
        };
        
        auto langAdapter = std::make_shared<StringListAdapter>(languages);
        langCombo->setAdapter(langAdapter);
        langCombo->setOnSelectionChanged([languages](size_t index) {
            if (index < languages.size()) {
                std::println("Language selected: {}", 
                    std::string(languages[index].begin(), languages[index].end()));
            }
        });
        container->addChild(langCombo);
        
        window->setRootWidget(container);
        window->show();
        
        std::println("✓ ComboBox demo ready");
        std::println("  Try selecting different options");
        
        app.run();
        return 0;
        
    } catch (const std::exception& e) {
        std::println(stderr, "ERROR: {}", e.what());
        return 1;
    }
}