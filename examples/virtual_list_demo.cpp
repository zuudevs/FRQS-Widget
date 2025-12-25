#include <frqs-widget.hpp>

using namespace frqs::widget;
using namespace frqs::core;

// 1. Buat Adapter Sederhana (Data Source)
class BigDataAdapter : public IListAdapter {
private:
    size_t count_ = 100000; // 100 Ribu Item!

public:
    // Pastikan override virtual functions dari IListAdapter
    size_t getCount() const override { 
        return count_; 
    }

    std::shared_ptr<IWidget> createView(size_t) override {
        // Recycle: Membuat widget baru HANYA jika pool kosong
        auto label = std::make_shared<Label>();
        label->setPadding(10);
        return label;
    }

    void updateView(size_t index, IWidget* view) override {
        // Rebind: Mengupdate data widget yang di-recycle
        if (auto* label = dynamic_cast<Label*>(view)) {
            // Gunakan std::format untuk performa
            label->setText(std::format(L"Item Data #{} - High Performance Row", index));
            
            // Visualisasi Recycling: Ganti warna selang-seling
            label->setBackgroundColor(
                index % 2 == 0 ? Color(245, 245, 245) : Color(255, 255, 255)
            );
        }
    }
};

int main() {
    auto& app = Application::instance();
	app.initialize();
    
    WindowParams params ;
	params.title = L"FRQS Virtual List Demo (100k Items)";
	params.size = Size(800u, 600u);
	params.position = Point(100, 100);
    auto window = app.createWindow(params);
    
    auto listView = std::make_shared<ListView>();
    
    listView->setRect({0, 0, 800, 600});
    listView->setItemHeight(35); // Tinggi fixed per item
    
    auto adapter = std::make_shared<BigDataAdapter>();
    listView->setAdapter(adapter);
    
    listView->setOnSelectionChanged([](size_t index) {
        // Callback saat user klik item
        // Debug output bisa ditaruh disini
    });

    window->setRootWidget(listView);
    window->show();
    
    app.run();
}