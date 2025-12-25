/**
 * @file virtual_list_demo.cpp
 * @brief Virtual ListView with large dataset demonstration
 */

#include "frqs-widget.hpp"
#include "widget/list_view.hpp"
#include "widget/list_adapter.hpp"
#include "widget/label.hpp"
#include <print>
#include <vector>
#include <string>

using namespace frqs;

// Simple data model
struct Person {
    std::wstring name;
    int age;
    std::wstring city;
};

// Custom adapter for Person list
class PersonListAdapter : public widget::IListAdapter {
private:
    std::vector<Person> people_;
    
public:
    explicit PersonListAdapter(std::vector<Person> people)
        : people_(std::move(people)) {}
    
    size_t getCount() const override {
        return people_.size();
    }
    
    std::shared_ptr<widget::IWidget> createView(size_t) override {
        auto container = std::make_shared<widget::Widget>();
        container->setBackgroundColor(widget::colors::White);
        
        // Name label
        auto nameLabel = std::make_shared<widget::Label>();
        nameLabel->setRect(widget::Rect(10, 5, 200u, 25u));
        nameLabel->setFontBold(true);
        container->addChild(nameLabel);
        
        // Info label
        auto infoLabel = std::make_shared<widget::Label>();
        infoLabel->setRect(widget::Rect(220, 5, 300u, 25u));
        infoLabel->setTextColor(widget::Color(100, 100, 100));
        container->addChild(infoLabel);
        
        return container;
    }
    
    void updateView(size_t index, widget::IWidget* view) override {
        if (index >= people_.size()) return;
        
        const auto& person = people_[index];
        auto* container = dynamic_cast<widget::Widget*>(view);
        if (!container) return;
        
        const auto& children = container->getChildren();
        if (children.size() >= 2) {
            if (auto* nameLabel = dynamic_cast<widget::Label*>(children[0].get())) {
                nameLabel->setText(person.name);
            }
            if (auto* infoLabel = dynamic_cast<widget::Label*>(children[1].get())) {
                infoLabel->setText(std::format(L"Age: {} | City: {}", 
                    person.age, person.city));
            }
        }
        
        // Alternate row colors
        if (index % 2 == 0) {
            container->setBackgroundColor(widget::Color(250, 250, 250));
        } else {
            container->setBackgroundColor(widget::colors::White);
        }
    }
};

int main() {
    try {
        std::println("=== Virtual List Demo ===\n");
        
        auto& app = Application::instance();
        app.initialize();
        
        WindowParams params;
        params.title = L"Virtual List Demo (1000 items)";
        params.size = widget::Size(600u, 500u);
        auto window = app.createWindow(params);
        
        // Generate test data
        std::vector<Person> people;
        std::vector<std::wstring> cities = {
            L"Jakarta", L"Surabaya", L"Bandung", L"Medan", L"Semarang"
        };
        
        for (int i = 1; i <= 1000; i++) {
            people.push_back({
                std::format(L"Person {}", i),
                20 + (i % 50),
                cities[i % cities.size()]
            });
        }
        
        std::println("Generated {} test records", people.size());
        
        // Create ListView
        auto listView = std::make_shared<widget::ListView>();
        listView->setRect(window->getClientRect());
        listView->setBackgroundColor(widget::colors::White);
        listView->setItemHeight(35);
        
        // Set adapter
        auto adapter = std::make_shared<PersonListAdapter>(std::move(people));
        listView->setAdapter(adapter);
        
        // Handle selection
        listView->setOnSelectionChanged([](size_t index) {
            std::println("Selected item at index: {}", index);
        });
        
        window->setRootWidget(listView);
        window->show();
        
        std::println("✓ Virtual list demo ready");
        std::println("  Items: 1000");
        std::println("  Only visible items are rendered!");
        std::println("  Scroll to test performance");
        
        app.run();
        return 0;
        
    } catch (const std::exception& e) {
        std::println(stderr, "ERROR: {}", e.what());
        return 1;
    }
}