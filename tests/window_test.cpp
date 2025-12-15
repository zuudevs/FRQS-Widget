// tests/window_test.cpp - Window Unit Tests
#include "frqs-widget.hpp"
#include <print>
#include <cassert>
#include <thread>

using namespace frqs;

// ============================================================================
// TEST HELPERS
// ============================================================================

#define TEST(name) \
    void test_##name(); \
    struct TestRegistrar_##name { \
        TestRegistrar_##name() { \
            std::println("Running test: {}", #name); \
            test_##name(); \
            std::println("✓ Test passed: {}\n", #name); \
        } \
    } g_testRegistrar_##name; \
    void test_##name()

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        std::println(stderr, "Assertion failed: {} != {}", #a, #b); \
        std::terminate(); \
    }

#define ASSERT_TRUE(expr) \
    if (!(expr)) { \
        std::println(stderr, "Assertion failed: {}", #expr); \
        std::terminate(); \
    }

// ============================================================================
// WINDOW TESTS
// ============================================================================

TEST(window_creation) {
    auto& app = Application::instance();
    app.initialize();

    WindowParams params;
    params.title = L"Test Window";
    params.size = widget::Size(640u, 480u);
    params.position = widget::Point(50, 50);
    params.visible = false;  // Don't show during test

    auto window = app.createWindow(params);
    
    ASSERT_TRUE(window != nullptr);
    ASSERT_EQ(window->getTitle(), L"Test Window");
    ASSERT_EQ(window->getSize().w, 640u);
    ASSERT_EQ(window->getSize().h, 480u);
    ASSERT_EQ(window->isVisible(), false);

    window->close();
}

TEST(window_visibility) {
    auto& app = Application::instance();
    
    WindowParams params;
    params.visible = false;
    
    auto window = app.createWindow(params);
    
    ASSERT_EQ(window->isVisible(), false);
    
    window->show();
    ASSERT_EQ(window->isVisible(), true);
    
    window->hide();
    ASSERT_EQ(window->isVisible(), false);
    
    window->close();
}

TEST(window_resize) {
    auto& app = Application::instance();
    
    auto window = app.createWindow();
    
    widget::Size newSize(1024u, 768u);
    window->setSize(newSize);
    
    auto actualSize = window->getSize();
    ASSERT_EQ(actualSize.w, newSize.w);
    ASSERT_EQ(actualSize.h, newSize.h);
    
    window->close();
}

TEST(window_position) {
    auto& app = Application::instance();
    
    auto window = app.createWindow();
    
    widget::Point newPos(200, 100);
    window->setPosition(newPos);
    
    auto actualPos = window->getPosition();
    ASSERT_EQ(actualPos.x, newPos.x);
    ASSERT_EQ(actualPos.y, newPos.y);
    
    window->close();
}

TEST(window_client_rect) {
    auto& app = Application::instance();
    
    WindowParams params;
    params.size = widget::Size(800u, 600u);
    
    auto window = app.createWindow(params);
    
    auto clientRect = window->getClientRect();
    ASSERT_EQ(clientRect.x, 0);
    ASSERT_EQ(clientRect.y, 0);
    ASSERT_EQ(clientRect.w, 800u);
    ASSERT_EQ(clientRect.h, 600u);
    
    window->close();
}

TEST(window_widget_tree) {
    auto& app = Application::instance();
    
    auto window = app.createWindow();
    
    auto rootWidget = std::make_shared<widget::Widget>();
    window->setRootWidget(rootWidget);
    
    auto retrievedRoot = window->getRootWidget();
    ASSERT_TRUE(retrievedRoot != nullptr);
    ASSERT_TRUE(retrievedRoot.get() == rootWidget.get());
    
    window->close();
}

TEST(window_registry) {
    auto& app = Application::instance();
    auto& registry = app.getRegistry();
    
    size_t initialCount = registry.getWindowCount();
    
    auto window1 = app.createWindow();
    auto window2 = app.createWindow();
    
    ASSERT_EQ(registry.getWindowCount(), initialCount + 2);
    
    auto id1 = window1->getId();
    auto found = registry.getWindow(id1);
    
    ASSERT_TRUE(found.has_value());
    ASSERT_TRUE(found.value().get() == window1.get());
    
    window1->close();
    window2->close();
    
    ASSERT_EQ(registry.getWindowCount(), initialCount);
}

TEST(multiple_windows) {
    auto& app = Application::instance();
    
    std::vector<std::shared_ptr<Window>> windows;
    
    for (int i = 0; i < 5; ++i) {
        WindowParams params;
        params.title = std::format(L"Window {}", i);
        params.position = widget::Point(100 + i * 50, 100 + i * 50);
        params.visible = false;
        
        windows.push_back(app.createWindow(params));
    }
    
    ASSERT_EQ(windows.size(), 5);
    
    for (auto& window : windows) {
        ASSERT_TRUE(window != nullptr);
        window->close();
    }
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::println("=== FRQS-Widget Window Tests ===\n");
    
    try {
        // Tests are automatically registered and run by static initializers
        std::println("\n=== All tests passed! ===");
        return 0;
        
    } catch (const std::exception& e) {
        std::println(stderr, "\nTest failed with exception: {}", e.what());
        return 1;
    }
}