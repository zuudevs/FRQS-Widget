#include "widget/label.hpp"
#include <frqs-widget.hpp>

using namespace frqs::core;
using namespace frqs::widget;

int main() {
    auto& app = Application::instance();
	app.initialize();

	WindowParams params;
	params.title = L"FRQS Widget Demo";
	params.size = Size(800u, 600u);
	params.position = Point(100, 100);
    auto window = app.createWindow(params);   
    
    auto label = std::make_shared<Label>(L"Welcome to FRQS Widget!");
    label->setAlignment(Label::Alignment::Center);
    
    window->setRootWidget(label);
    window->show();
    
    app.run();
}