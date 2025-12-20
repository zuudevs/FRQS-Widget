// examples/kitchen_sink_demo.cpp - Complete Widget Showcase
#include "frqs-widget.hpp"
#include "widget/container.hpp"
#include "widget/label.hpp"
#include "widget/button.hpp"
#include "widget/text_input.hpp"
#include "widget/slider.hpp"
#include "widget/checkbox.hpp"
#include "widget/combobox.hpp"
#include "widget/list_view.hpp"
#include "widget/list_adapter.hpp"
#include "widget/scroll_view.hpp"
#include "widget/image.hpp"
#include <print>
#include <format>

using namespace frqs;
using namespace frqs::widget;

// ============================================================================
// SIMPLE STRING ADAPTER
// ============================================================================

class StringAdapter : public IListAdapter {
private:
    std::vector<std::wstring> items_;
    
public:
    explicit StringAdapter(std::vector<std::wstring> items)
        : items_(std::move(items)) {}
    
    size_t getCount() const override {
        return items_.size();
    }
    
    std::shared_ptr<IWidget> createView(size_t) override {
        auto label = std::make_shared<Label>();
        label->setAlignment(Label::Alignment::Left);
        label->setVerticalAlignment(Label::VerticalAlignment::Middle);
        label->setPadding(10);
        return label;
    }
    
    void updateView(size_t index, IWidget* view) override {
        if (index >= items_.size()) return;
        
        auto* label = dynamic_cast<Label*>(view);
        if (!label) return;
        
        label->setText(items_[index]);
        
        // Alternating colors
        if (index % 2 == 0) {
            label->setBackgroundColor(colors::White);
        } else {
            label->setBackgroundColor(Color(250, 250, 250));
        }
    }
};

// ============================================================================
// KITCHEN SINK DEMO
// ============================================================================

class KitchenSinkDemo {
private:
    std::shared_ptr<Label> statusLabel_;
    std::shared_ptr<Label> counterLabel_;
    int counter_ = 0;

public:
    void run() {
        try {
            std::println("=== FRQS-Widget: Kitchen Sink Demo ===");
            std::println("");
            std::println("🎨 Complete Widget Showcase");
            std::println("Demonstrating ALL widgets in one place!");
            std::println("");
            
            auto& app = Application::instance();
            app.initialize();
            
            // Create main window
            WindowParams params;
            params.title = L"Kitchen Sink - All Widgets Demo";
            params.size = Size(1200u, 800u);
            params.position = Point(50, 50);
            
            auto window = app.createWindow(params);
            
            // Build UI
            buildUI(window);
            
            window->show();
            
            std::println("✓ Demo window created");
            std::println("");
            std::println("Features shown:");
            std::println("  ✓ Buttons (Primary, Success, Danger, Disabled)");
            std::println("  ✓ Text Input (with placeholder, selection)");
            std::println("  ✓ Sliders (Horizontal & Vertical)");
            std::println("  ✓ CheckBoxes (Interactive toggles)");
            std::println("  ✓ ComboBox (Dropdown selector)");
            std::println("  ✓ ListView (Virtual scrolling)");
            std::println("  ✓ ScrollView (Draggable scrollbars)");
            std::println("  ✓ Labels (Various alignments)");
            std::println("  ✓ Containers (FlexLayout system)");
            std::println("  ✓ Images (Scale modes)");
            std::println("");
            std::println("Close the window to exit.\n");
            
            app.run();
            
            std::println("\nDemo ended successfully.");
            
        } catch (const std::exception& e) {
            std::println(stderr, "ERROR: {}", e.what());
        }
    }

private:
    void buildUI(std::shared_ptr<core::Window> window) {
        // Root: Scrollable container
        auto scrollView = std::make_shared<ScrollView>();
        scrollView->setRect(window->getClientRect());
        scrollView->setBackgroundColor(Color(245, 245, 245));
        
        // Content container
        auto content = createFlexColumn(20, 20);
        content->setBackgroundColor(Color(245, 245, 245));
        
        // ====================================================================
        // HEADER
        // ====================================================================
        auto header = std::make_shared<Label>(L"🎨 FRQS-Widget Kitchen Sink");
        header->setLayoutWeight(0.0f);
        header->setRect(Rect(0, 0, 1160u, 80u));
        header->setBackgroundColor(Color(52, 152, 219));
        header->setTextColor(colors::White);
        header->setFontSize(28.0f);
        header->setFontBold(true);
        header->setAlignment(Label::Alignment::Center);
        header->setVerticalAlignment(Label::VerticalAlignment::Middle);
        content->addChild(header);
        
        // ====================================================================
        // BUTTONS SECTION
        // ====================================================================
        content->addChild(createSectionTitle(L"🖱️ Buttons"));
        content->addChild(createButtonsSection());
        
        // ====================================================================
        // TEXT INPUT SECTION
        // ====================================================================
        content->addChild(createSectionTitle(L"⌨️ Text Input"));
        content->addChild(createTextInputSection());
        
        // ====================================================================
        // SLIDERS SECTION
        // ====================================================================
        content->addChild(createSectionTitle(L"🎚️ Sliders"));
        content->addChild(createSlidersSection());
        
        // ====================================================================
        // CHECKBOXES SECTION
        // ====================================================================
        content->addChild(createSectionTitle(L"☑️ CheckBoxes"));
        content->addChild(createCheckBoxSection());
        
        // ====================================================================
        // COMBOBOX SECTION
        // ====================================================================
        content->addChild(createSectionTitle(L"📋 ComboBox"));
        content->addChild(createComboBoxSection());
        
        // ====================================================================
        // LISTVIEW SECTION
        // ====================================================================
        content->addChild(createSectionTitle(L"📜 ListView (Virtual)"));
        content->addChild(createListViewSection());
        
        // ====================================================================
        // LABELS SECTION
        // ====================================================================
        content->addChild(createSectionTitle(L"🏷️ Labels"));
        content->addChild(createLabelsSection());
        
        // ====================================================================
        // STATUS BAR
        // ====================================================================
        statusLabel_ = std::make_shared<Label>(
            L"🚀 All widgets loaded successfully! Interact with any widget to see updates."
        );
        statusLabel_->setLayoutWeight(0.0f);
        statusLabel_->setRect(Rect(0, 0, 1160u, 50u));
        statusLabel_->setBackgroundColor(Color(46, 204, 113));
        statusLabel_->setTextColor(colors::White);
        statusLabel_->setFontSize(14.0f);
        statusLabel_->setPadding(15);
        statusLabel_->setVerticalAlignment(Label::VerticalAlignment::Middle);
        content->addChild(statusLabel_);
        
        // Calculate total content height
        uint32_t totalHeight = 80 + 20  // header
                             + 50 + 20  // buttons title
                             + 200 + 20  // buttons section
                             + 50 + 20  // text input title
                             + 150 + 20  // text input section
                             + 50 + 20  // sliders title
                             + 200 + 20  // sliders section
                             + 50 + 20  // checkbox title
                             + 150 + 20  // checkbox section
                             + 50 + 20  // combobox title
                             + 100 + 20  // combobox section
                             + 50 + 20  // listview title
                             + 250 + 20  // listview section
                             + 50 + 20  // labels title
                             + 150 + 20  // labels section
                             + 50 + 40;  // status bar + padding
        
        content->setRect(Rect(0, 0, 1160u, totalHeight));
        
        scrollView->setContent(content);
        window->setRootWidget(scrollView);
    }
    
    std::shared_ptr<Label> createSectionTitle(const std::wstring& title) {
        auto label = std::make_shared<Label>(title);
        label->setLayoutWeight(0.0f);
        label->setRect(Rect(0, 0, 1160u, 50u));
        label->setBackgroundColor(Color(52, 73, 94));
        label->setTextColor(colors::White);
        label->setFontSize(18.0f);
        label->setFontBold(true);
        label->setPadding(15);
        label->setVerticalAlignment(Label::VerticalAlignment::Middle);
        return label;
    }
    
    std::shared_ptr<Container> createButtonsSection() {
        auto section = createFlexColumn(10, 15);
        section->setLayoutWeight(0.0f);
        section->setRect(Rect(0, 0, 1160u, 200u));
        section->setBackgroundColor(colors::White);
        section->setBorder(Color(220, 220, 220), 1.0f);
        
        // Description
        auto desc = std::make_shared<Label>(
            L"Buttons with different states and colors"
        );
        desc->setLayoutWeight(0.0f);
        desc->setTextColor(Color(127, 140, 141));
        section->addChild(desc);
        
        // Button row 1
        auto row1 = createFlexRow(10, 0);
        row1->setLayoutWeight(0.0f);
        
        auto btnPrimary = std::make_shared<Button>(L"Primary");
        btnPrimary->setLayoutWeight(1.0f);
        btnPrimary->setNormalColor(Color(52, 152, 219));
        btnPrimary->setOnClick([this]() {
            updateStatus(L"Primary button clicked!");
        });
        row1->addChild(btnPrimary);
        
        auto btnSuccess = std::make_shared<Button>(L"Success");
        btnSuccess->setLayoutWeight(1.0f);
        btnSuccess->setNormalColor(Color(46, 204, 113));
        btnSuccess->setOnClick([this]() {
            updateStatus(L"Success!");
        });
        row1->addChild(btnSuccess);
        
        auto btnDanger = std::make_shared<Button>(L"Danger");
        btnDanger->setLayoutWeight(1.0f);
        btnDanger->setNormalColor(Color(231, 76, 60));
        btnDanger->setOnClick([this]() {
            updateStatus(L"⚠️ Danger button clicked!");
        });
        row1->addChild(btnDanger);
        
        auto btnDisabled = std::make_shared<Button>(L"Disabled");
        btnDisabled->setLayoutWeight(1.0f);
        btnDisabled->setEnabled(false);
        row1->addChild(btnDisabled);
        
        section->addChild(row1);
        
        // Counter button
        auto counterRow = createFlexRow(10, 0);
        counterRow->setLayoutWeight(0.0f);
        
        counterLabel_ = std::make_shared<Label>(L"Counter: 0");
        counterLabel_->setLayoutWeight(1.0f);
        counterLabel_->setBackgroundColor(Color(236, 240, 241));
        counterLabel_->setPadding(10);
        counterLabel_->setFontSize(16.0f);
        counterLabel_->setFontBold(true);
        counterLabel_->setAlignment(Label::Alignment::Center);
        counterLabel_->setVerticalAlignment(Label::VerticalAlignment::Middle);
        counterRow->addChild(counterLabel_);
        
        auto btnIncrement = std::make_shared<Button>(L"➕ Increment");
        btnIncrement->setLayoutWeight(0.0f);
        btnIncrement->setRect(Rect(0, 0, 150u, 50u));
        btnIncrement->setNormalColor(Color(155, 89, 182));
        btnIncrement->setOnClick([this]() {
            counter_++;
            counterLabel_->setText(std::format(L"Counter: {}", counter_));
            updateStatus(std::format(L"Counter increased to {}", counter_));
        });
        counterRow->addChild(btnIncrement);
        
        section->addChild(counterRow);
        
        return section;
    }
    
    std::shared_ptr<Container> createTextInputSection() {
        auto section = createFlexColumn(10, 15);
        section->setLayoutWeight(0.0f);
        section->setRect(Rect(0, 0, 1160u, 150u));
        section->setBackgroundColor(colors::White);
        section->setBorder(Color(220, 220, 220), 1.0f);
        
        auto desc = std::make_shared<Label>(
            L"Text input with placeholder and live feedback"
        );
        desc->setLayoutWeight(0.0f);
        desc->setTextColor(Color(127, 140, 141));
        section->addChild(desc);
        
        auto textInput = std::make_shared<TextInput>();
        textInput->setLayoutWeight(0.0f);
        textInput->setRect(Rect(0, 0, 1130u, 45u));
        textInput->setPlaceholder(L"Type something here...");
        textInput->setOnTextChanged([this](const std::wstring& text) {
            if (text.length() % 5 == 0 && !text.empty()) {
                updateStatus(std::format(L"Text length: {} characters", text.length()));
            }
        });
        textInput->setOnEnter([this](const std::wstring& text) {
            updateStatus(std::format(L"✓ Submitted: \"{}\"", text));
        });
        section->addChild(textInput);
        
        return section;
    }
    
    std::shared_ptr<Container> createSlidersSection() {
        auto section = createFlexColumn(15, 15);
        section->setLayoutWeight(0.0f);
        section->setRect(Rect(0, 0, 1160u, 200u));
        section->setBackgroundColor(colors::White);
        section->setBorder(Color(220, 220, 220), 1.0f);
        
        auto desc = std::make_shared<Label>(L"Sliders with real-time value updates");
        desc->setLayoutWeight(0.0f);
        desc->setTextColor(Color(127, 140, 141));
        section->addChild(desc);
        
        // Horizontal slider
        auto hSliderRow = createFlexRow(15, 0);
        hSliderRow->setLayoutWeight(0.0f);
        
        auto hLabel = std::make_shared<Label>(L"Volume: 50%");
        hLabel->setLayoutWeight(0.0f);
        hLabel->setRect(Rect(0, 0, 150u, 50u));
        hLabel->setFontBold(true);
        hSliderRow->addChild(hLabel);
        
        auto hSlider = std::make_shared<Slider>(Slider::Orientation::Horizontal);
        hSlider->setLayoutWeight(1.0f);
        hSlider->setRange(0, 100);
        hSlider->setValue(50);
        hSlider->setOnValueChanged([this, hLabel](double value) {
            hLabel->setText(std::format(L"Volume: {}%", static_cast<int>(value)));
            updateStatus(std::format(L"Volume: {}%", static_cast<int>(value)));
        });
        hSliderRow->addChild(hSlider);
        
        section->addChild(hSliderRow);
        
        return section;
    }
    
    std::shared_ptr<Container> createCheckBoxSection() {
        auto section = createFlexColumn(10, 15);
        section->setLayoutWeight(0.0f);
        section->setRect(Rect(0, 0, 1160u, 150u));
        section->setBackgroundColor(colors::White);
        section->setBorder(Color(220, 220, 220), 1.0f);
        
        auto desc = std::make_shared<Label>(L"Interactive checkboxes");
        desc->setLayoutWeight(0.0f);
        desc->setTextColor(Color(127, 140, 141));
        section->addChild(desc);
        
        auto cbRow = createFlexRow(20, 0);
        cbRow->setLayoutWeight(0.0f);
        
        auto cb1 = std::make_shared<CheckBox>(L"Feature A");
        cb1->setLayoutWeight(0.0f);
        cb1->setRect(Rect(0, 0, 200u, 35u));
        cb1->setChecked(true);
        cb1->setOnChanged([this](bool checked) {
            updateStatus(std::format(L"Feature A: {}", checked ? L"ON" : L"OFF"));
        });
        cbRow->addChild(cb1);
        
        auto cb2 = std::make_shared<CheckBox>(L"Feature B");
        cb2->setLayoutWeight(0.0f);
        cb2->setRect(Rect(0, 0, 200u, 35u));
        cb2->setOnChanged([this](bool checked) {
            updateStatus(std::format(L"Feature B: {}", checked ? L"ON" : L"OFF"));
        });
        cbRow->addChild(cb2);
        
        auto cb3 = std::make_shared<CheckBox>(L"Feature C (Disabled)");
        cb3->setLayoutWeight(0.0f);
        cb3->setRect(Rect(0, 0, 250u, 35u));
        cb3->setEnabled(false);
        cbRow->addChild(cb3);
        
        section->addChild(cbRow);
        
        return section;
    }
    
    std::shared_ptr<Container> createComboBoxSection() {
        auto section = createFlexColumn(10, 15);
        section->setLayoutWeight(0.0f);
        section->setRect(Rect(0, 0, 1160u, 100u));
        section->setBackgroundColor(colors::White);
        section->setBorder(Color(220, 220, 220), 1.0f);
        
        auto desc = std::make_shared<Label>(L"Dropdown selector");
        desc->setLayoutWeight(0.0f);
        desc->setTextColor(Color(127, 140, 141));
        section->addChild(desc);
        
        auto comboBox = std::make_shared<ComboBox>();
        comboBox->setLayoutWeight(0.0f);
        comboBox->setRect(Rect(0, 0, 400u, 40u));
        
        auto adapter = std::make_shared<StringAdapter>(std::vector<std::wstring>{
            L"Option 1", L"Option 2", L"Option 3", L"Option 4", L"Option 5"
        });
        comboBox->setAdapter(adapter);
        comboBox->setSelectedIndex(0);
        comboBox->setOnSelectionChanged([this](size_t idx) {
            updateStatus(std::format(L"Selected option {}", idx + 1));
        });
        
        section->addChild(comboBox);
        
        return section;
    }
    
    std::shared_ptr<Container> createListViewSection() {
        auto section = createFlexColumn(10, 15);
        section->setLayoutWeight(0.0f);
        section->setRect(Rect(0, 0, 1160u, 250u));
        section->setBackgroundColor(colors::White);
        section->setBorder(Color(220, 220, 220), 1.0f);
        
        auto desc = std::make_shared<Label>(
            L"Virtual scrolling list (100 items, only ~20 rendered)"
        );
        desc->setLayoutWeight(0.0f);
        desc->setTextColor(Color(127, 140, 141));
        section->addChild(desc);
        
        auto listView = std::make_shared<ListView>();
        listView->setLayoutWeight(1.0f);
        listView->setItemHeight(35);
        
        std::vector<std::wstring> items;
        for (int i = 0; i < 100; ++i) {
            items.push_back(std::format(L"List Item #{:03d}", i + 1));
        }
        
        auto adapter = std::make_shared<StringAdapter>(items);
        listView->setAdapter(adapter);
        listView->setOnSelectionChanged([this](size_t idx) {
            updateStatus(std::format(L"Selected item #{}", idx + 1));
        });
        
        section->addChild(listView);
        
        return section;
    }
    
    std::shared_ptr<Container> createLabelsSection() {
        auto section = createFlexColumn(10, 15);
        section->setLayoutWeight(0.0f);
        section->setRect(Rect(0, 0, 1160u, 150u));
        section->setBackgroundColor(colors::White);
        section->setBorder(Color(220, 220, 220), 1.0f);
        
        auto desc = std::make_shared<Label>(L"Labels with different alignments");
        desc->setLayoutWeight(0.0f);
        desc->setTextColor(Color(127, 140, 141));
        section->addChild(desc);
        
        auto leftLabel = std::make_shared<Label>(L"Left aligned text");
        leftLabel->setLayoutWeight(0.0f);
        leftLabel->setRect(Rect(0, 0, 1130u, 30u));
        leftLabel->setAlignment(Label::Alignment::Left);
        leftLabel->setBackgroundColor(Color(236, 240, 241));
        leftLabel->setPadding(8);
        section->addChild(leftLabel);
        
        auto centerLabel = std::make_shared<Label>(L"Center aligned text");
        centerLabel->setLayoutWeight(0.0f);
        centerLabel->setRect(Rect(0, 0, 1130u, 30u));
        centerLabel->setAlignment(Label::Alignment::Center);
        centerLabel->setBackgroundColor(Color(236, 240, 241));
        centerLabel->setPadding(8);
        section->addChild(centerLabel);
        
        auto rightLabel = std::make_shared<Label>(L"Right aligned text");
        rightLabel->setLayoutWeight(0.0f);
        rightLabel->setRect(Rect(0, 0, 1130u, 30u));
        rightLabel->setAlignment(Label::Alignment::Right);
        rightLabel->setBackgroundColor(Color(236, 240, 241));
        rightLabel->setPadding(8);
        section->addChild(rightLabel);
        
        return section;
    }
    
    void updateStatus(const std::wstring& message) {
        if (statusLabel_) {
            statusLabel_->setText(std::format(L"🔔 {}", message));
        }
        std::println("Status: {}", std::string(message.begin(), message.end()));
    }
};

// ============================================================================
// MAIN
// ============================================================================

int main() {
    KitchenSinkDemo demo;
    demo.run();
    return 0;
}