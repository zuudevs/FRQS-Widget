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
// SIMPLE STRING ADAPTER (Buat ListView)
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
        
        // Warna selang-seling biar cantik
        if (index % 2 == 0) {
            label->setBackgroundColor(colors::White);
        } else {
            label->setBackgroundColor(Color(250, 250, 250));
        }
    }
};

// ============================================================================
// KITCHEN SINK DEMO CLASS
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
            
            auto& app = Application::instance();
            app.initialize();
            
            // Bikin window utama
            WindowParams params;
            params.title = L"Kitchen Sink - All Widgets Demo";
            params.size = Size(1200u, 800u);
            params.position = Point(50, 50);
            
            auto window = app.createWindow(params);
            
            // Build semua UI
            buildUI(window);
            
            window->show();
            app.run();
            
        } catch (const std::exception& e) {
            std::println(stderr, "ERROR: {}", e.what());
        }
    }

private:
    void buildUI(std::shared_ptr<core::Window> window) {
        // Root: ScrollView biar konten panjang bisa discroll
        auto scrollView = std::make_shared<ScrollView>();
        scrollView->setRect(window->getClientRect());
        scrollView->setBackgroundColor(Color(245, 245, 245));
        
        // Content container dengan FlexLayout (Column)
        auto content = createFlexColumn(20, 20); // Spacing 20, Padding 20
        content->setBackgroundColor(Color(245, 245, 245));
        
        // Header
        auto header = std::make_shared<Label>(L"🎨 FRQS-Widget Kitchen Sink");
        header->setLayoutWeight(0.0f); // Fixed height
        header->setRect(Rect(0, 0, 1160u, 80u));
        header->setBackgroundColor(Color(52, 152, 219));
        header->setTextColor(colors::White);
        header->setFontSize(28.0f);
        header->setFontBold(true);
        header->setAlignment(Label::Alignment::Center);
        header->setVerticalAlignment(Label::VerticalAlignment::Middle);
        content->addChild(header);
        
        // Tambahkan semua seksi widget
        content->addChild(createSectionTitle(L"🖱️ Buttons"));
        content->addChild(createButtonsSection());
        
        content->addChild(createSectionTitle(L"⌨️ Text Input"));
        content->addChild(createTextInputSection());
        
        content->addChild(createSectionTitle(L"🎚️ Sliders"));
        content->addChild(createSlidersSection());
        
        content->addChild(createSectionTitle(L"☑️ CheckBoxes"));
        content->addChild(createCheckBoxSection());
        
        content->addChild(createSectionTitle(L"📋 ComboBox"));
        content->addChild(createComboBoxSection());
        
        content->addChild(createSectionTitle(L"📜 ListView (Virtual)"));
        content->addChild(createListViewSection());
        
        content->addChild(createSectionTitle(L"🏷️ Labels"));
        content->addChild(createLabelsSection());
        
        // Status Bar di bawah
        statusLabel_ = std::make_shared<Label>(
            L"🚀 All widgets loaded successfully! Interact with any widget to see updates."
        );
        statusLabel_->setLayoutWeight(0.0f);
        statusLabel_->setRect(Rect(0, 0, 1160u, 50u));
        statusLabel_->setBackgroundColor(Color(46, 204, 113));
        statusLabel_->setTextColor(colors::White);
        statusLabel_->setPadding(15);
        content->addChild(statusLabel_);
        
        // Hitung total tinggi konten untuk ScrollView
        // (Dalam implementasi layout otomatis, ini mungkin dihitung sendiri, 
        // tapi di sini kita set manual untuk ScrollView content)
        content->setRect(Rect(0, 0, 1160u, 1600u)); 
        
        scrollView->setContent(content);
        window->setRootWidget(scrollView);
    }
    
    // Helper untuk bikin judul seksi
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
        
        // Baris tombol
        auto row1 = createFlexRow(10, 0);
        row1->setLayoutWeight(0.0f);
        
        auto btnPrimary = std::make_shared<Button>(L"Primary");
        btnPrimary->setLayoutWeight(1.0f);
        btnPrimary->setNormalColor(Color(52, 152, 219));
        btnPrimary->setOnClick([this]() { updateStatus(L"Primary button clicked!"); });
        row1->addChild(btnPrimary);
        
        auto btnDanger = std::make_shared<Button>(L"Danger");
        btnDanger->setLayoutWeight(1.0f);
        btnDanger->setNormalColor(Color(231, 76, 60));
        btnDanger->setOnClick([this]() { updateStatus(L"⚠️ Danger button clicked!"); });
        row1->addChild(btnDanger);

        section->addChild(row1);
        
        // Baris Counter
        auto counterRow = createFlexRow(10, 0);
        counterRow->setLayoutWeight(0.0f);
        
        counterLabel_ = std::make_shared<Label>(L"Counter: 0");
        counterLabel_->setLayoutWeight(1.0f);
        counterLabel_->setBackgroundColor(Color(236, 240, 241));
        counterLabel_->setAlignment(Label::Alignment::Center);
        counterRow->addChild(counterLabel_);
        
        auto btnIncrement = std::make_shared<Button>(L"➕ Increment");
        btnIncrement->setLayoutWeight(0.0f);
        btnIncrement->setRect(Rect(0, 0, 150u, 50u));
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
        section->setRect(Rect(0, 0, 1160u, 100u));
        section->setBackgroundColor(colors::White);
        
        auto textInput = std::make_shared<TextInput>();
        textInput->setLayoutWeight(0.0f);
        textInput->setRect(Rect(0, 0, 1130u, 45u));
        textInput->setPlaceholder(L"Type something here...");
        textInput->setOnEnter([this](const std::wstring& text) {
            updateStatus(std::format(L"✓ Submitted: \"{}\"", text));
        });
        section->addChild(textInput);
        return section;
    }

    std::shared_ptr<Container> createSlidersSection() {
        auto section = createFlexColumn(15, 15);
        section->setLayoutWeight(0.0f);
        section->setRect(Rect(0, 0, 1160u, 100u));
        section->setBackgroundColor(colors::White);

        auto hSliderRow = createFlexRow(15, 0);
        auto hLabel = std::make_shared<Label>(L"Value: 50");
        hLabel->setRect(Rect(0,0,100u,50u));
        
        auto hSlider = std::make_shared<Slider>(Slider::Orientation::Horizontal);
        hSlider->setLayoutWeight(1.0f);
        hSlider->setRange(0, 100);
        hSlider->setValue(50);
        hSlider->setOnValueChanged([this, hLabel](double value) {
            hLabel->setText(std::format(L"Value: {}", static_cast<int>(value)));
        });
        
        hSliderRow->addChild(hLabel);
        hSliderRow->addChild(hSlider);
        section->addChild(hSliderRow);
        return section;
    }

    std::shared_ptr<Container> createCheckBoxSection() {
        auto section = createFlexColumn(10, 15);
        section->setLayoutWeight(0.0f);
        section->setRect(Rect(0, 0, 1160u, 100u));
        section->setBackgroundColor(colors::White);
        
        auto cb1 = std::make_shared<CheckBox>(L"Aktifkan Fitur Rahasia");
        cb1->setRect(Rect(0, 0, 300u, 35u));
        cb1->setOnChanged([this](bool checked) {
            updateStatus(std::format(L"Fitur Rahasia: {}", checked ? L"ON" : L"OFF"));
        });
        section->addChild(cb1);
        return section;
    }

    std::shared_ptr<Container> createComboBoxSection() {
        auto section = createFlexColumn(10, 15);
        section->setLayoutWeight(0.0f);
        section->setRect(Rect(0, 0, 1160u, 100u));
        section->setBackgroundColor(colors::White);
        
        auto comboBox = std::make_shared<ComboBox>();
        comboBox->setRect(Rect(0, 0, 400u, 40u));
        comboBox->setAdapter(std::make_shared<StringAdapter>(std::vector<std::wstring>{
            L"Pilihan 1", L"Pilihan 2", L"Pilihan 3"
        }));
        comboBox->setOnSelectionChanged([this](size_t idx) {
            updateStatus(std::format(L"Selected option {}", idx + 1));
        });
        section->addChild(comboBox);
        return section;
    }

    std::shared_ptr<Container> createListViewSection() {
        auto section = createFlexColumn(10, 15);
        section->setLayoutWeight(0.0f);
        section->setRect(Rect(0, 0, 1160u, 250u)); // Tinggi
        section->setBackgroundColor(colors::White);
        
        auto listView = std::make_shared<ListView>();
        listView->setLayoutWeight(1.0f);
        listView->setItemHeight(35);
        
        // Generate 100 item
        std::vector<std::wstring> items;
        for (int i = 0; i < 100; ++i) items.push_back(std::format(L"Item Data #{}", i + 1));
        
        listView->setAdapter(std::make_shared<StringAdapter>(items));
        section->addChild(listView);
        return section;
    }

    std::shared_ptr<Container> createLabelsSection() {
        auto section = createFlexColumn(10, 15);
        section->setRect(Rect(0, 0, 1160u, 100u));
        section->setBackgroundColor(colors::White);
        
        auto lbl = std::make_shared<Label>(L"Contoh Label Rata Kanan");
        lbl->setRect(Rect(0,0,1130u, 30u));
        lbl->setAlignment(Label::Alignment::Right);
        section->addChild(lbl);
        return section;
    }

    void updateStatus(const std::wstring& message) {
        if (statusLabel_) statusLabel_->setText(L"🔔 " + message);
        std::println("Status: {}", std::string(message.begin(), message.end()));
    }
};

int main() {
    KitchenSinkDemo demo;
    demo.run();
    return 0;
}