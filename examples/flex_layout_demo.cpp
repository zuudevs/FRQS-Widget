#include "core/window.hpp"
#include "widget/container.hpp"
#include <frqs-widget.hpp>

using namespace frqs::widget;
using namespace frqs::core;

int main() {
    auto& app = Application::instance();
	app.initialize();

	WindowParams params;
	params.title = L"FRQS Widget Demo";
	params.size = Size(800u, 600u);
	auto window = app.createWindow(params);

    auto root = std::make_shared<Container>();
    // Layout baris (Horizontal)
    root->setLayout(std::make_unique<FlexLayout>(FlexLayout::Direction::Row));
    
    // Kolom 1 (Merah)
    auto col1 = std::make_shared<Container>();
    col1->setBackgroundColor(Color(231, 76, 60));
	col1->setMaxSize(100, 0);
    
    // Kolom 2 (Biru - Flex Grow)
    auto col2 = std::make_shared<Container>();
    col2->setBackgroundColor(Color(52, 152, 219));
    // Asumsi: Container otomatis fill space yang tersisa di layout row default
    
    // Kolom 3 (Hijau)
    auto col3 = std::make_shared<Container>();
    col3->setBackgroundColor(Color(46, 204, 113));
    col3->setMaxSize(100, 0);

    root->addChild(col1);
    root->addChild(col2);
    root->addChild(col3);

    window->setRootWidget(root);
    window->show();
    app.run();
}