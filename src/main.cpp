// src/main.cpp - Hello Window Example
#include "frqs-widget.hpp" //
#include <print>

using namespace frqs;

// ============================================================================
// SIMPLE COLORED WIDGET EXAMPLE
// ============================================================================

class ColoredWidget : public widget::Widget {
private:
    widget::Color color_;
    std::wstring label_;

public:
    ColoredWidget(const widget::Color& color, const std::wstring& label = L"")
        : color_(color), label_(label) {
        setBackgroundColor(color);
    }

    void render(widget::Renderer& renderer) override {
        // [FIX 1] Panggil Base Render DULUAN!
        // Ini akan menggambar background (sesuai setBackgroundColor) dan children.
        // Kalau ditaruh di akhir, background akan menimpa gambar custom kita (Overpainting).
        Widget::render(renderer);
        
        auto rect = getRect();
        
        // [FIX 2] Gambar dekorasi di ATAS background/children
        
        // Draw border (Hitam)
        renderer.drawRect(rect, widget::colors::Black, 2.0f);
        
        // Draw label if present
        if (!label_.empty()) {
            // [FIX 3] Logika warna teks otomatis.
            // Kalau widgetnya Header/Footer (background gelap), teks Putih.
            // Kalau Root/Content (background terang), teks Hitam.
            widget::Color textColor = (label_ == L"Header" || label_ == L"Footer") 
                                      ? widget::colors::White 
                                      : widget::colors::Black;
                                      
            renderer.drawText(label_, rect, textColor);
        }
    }
};

// ============================================================================
// MAIN APPLICATION
// ============================================================================

int main() {
    try {
        std::println("=== FRQS-Widget Demo Application ===\n");
        
        // Get application instance
        auto& app = Application::instance();
        
        // Initialize application
        std::println("Initializing application...");
        app.initialize();
        
        // Create main window
        std::println("Creating main window...");
        WindowParams params;
        params.title = L"FRQS-Widget Demo";
        params.size = widget::Size(800u, 600u);
        params.position = widget::Point(100, 100);
        params.resizable = true;
        params.visible = true;
        params.decorated = true;
        
        auto mainWindow = app.createWindow(params);
        std::println("✓ Window created successfully");
        
        // Create root widget
        auto root = std::make_shared<ColoredWidget>(
            widget::Color(240, 240, 245), // Light Gray
            L"Root Widget"
        );
        root->setRect(mainWindow->getClientRect());
        
        // Create some child widgets
        auto header = std::make_shared<ColoredWidget>(
            widget::Color(41, 128, 185), // Blue
            L"Header"
        );
        header->setRect(widget::Rect(0, 0, 800u, 80u));
        
        auto content = std::make_shared<ColoredWidget>(
            widget::Color(236, 240, 241), // White Smoke
            L"Content Area"
        );
        content->setRect(widget::Rect(0, 80, 800u, 440u));
        
        auto footer = std::make_shared<ColoredWidget>(
            widget::Color(52, 73, 94), // Dark Blue
            L"Footer"
        );
        footer->setRect(widget::Rect(0, 520, 800u, 80u));
        
        // Build widget tree
        root->addChild(header);
        root->addChild(content);
        root->addChild(footer);
        
        // Set root widget to window
        mainWindow->setRootWidget(root);
        
        std::println("✓ Widget tree created\n");
        std::println("Application running...");
        std::println("Close the window to exit.\n");
        
        // Run event loop
        app.run();
        
        std::println("\nApplication terminated successfully.");
        return 0;
        
    } catch (const std::exception& e) {
        std::println(stderr, "ERROR: {}", e.what());
        return 1;
    }
}