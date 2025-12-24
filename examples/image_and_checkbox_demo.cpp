#include "widget/checkbox.hpp"
#include "widget/container.hpp"
#include "widget/image.hpp"
#include <frqs-widget.hpp>

using namespace frqs::widget;
using namespace frqs::core;

int main() {
    auto& app = Application::instance();
	app.initialize();

	WindowParams params;
	params.title = L"Resource Cache & WIC Demo";
	params.size = Size(600, 400);
	params.position = Point(100, 100);
    auto window = app.createWindow(params);

    auto container = std::make_shared<Container>();
    container->setLayout(std::make_unique<FlexLayout>(FlexLayout::Direction::Column));
    container->setPadding(20);

    // 1. Load Gambar (Pastikan ada file test.png di sebelah exe atau path absolute)
    // ResourceCache akan otomatis menduplikasi referensi texture di GPU
    auto img1 = std::make_shared<Image>(L"test_image.png"); // Ganti path sesuai filemu
	img1->setMaxSize(100, 100);
    
    auto img2 = std::make_shared<Image>(L"test_image.png"); // Path SAMA = Texture SAMA (Hemat RAM)
    img2->setMaxSize(100, 100);

    // 2. Checkbox Control
    auto chk = std::make_shared<CheckBox>(L"Enable High Quality Rendering");
    chk->setChecked(true);
    
    chk->setOnChanged([img1, img2](bool checked) {
        float opacity = checked ? 1.0f : 0.5f;
        img1->setOpacity(opacity);
        img2->setOpacity(opacity);
    });

    container->addChild(img1);
    container->addChild(chk);
    container->addChild(img2);

    window->setRootWidget(container);
    window->show();
    app.run();
}