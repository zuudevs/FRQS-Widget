// tests/event_test.cpp - Event System Unit Tests
#include "frqs-widget.hpp"
#include <print>
#include <cassert>
#include <chrono>

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
// EVENT VARIANT TESTS
// ============================================================================

TEST(event_variant_size) {
    using namespace event;
    
    // Verify event variant size is reasonable
    std::println("Event variant size: {} bytes", sizeof(Event));
    ASSERT_TRUE(sizeof(Event) <= 64);  // Should be <= 64 bytes
    
    // Individual event sizes
    std::println("MouseMoveEvent: {} bytes", sizeof(MouseMoveEvent));
    std::println("MouseButtonEvent: {} bytes", sizeof(MouseButtonEvent));
    std::println("KeyEvent: {} bytes", sizeof(KeyEvent));
    std::println("ResizeEvent: {} bytes", sizeof(ResizeEvent));
    std::println("PaintEvent: {} bytes", sizeof(PaintEvent));
    std::println("FileDropEvent: {} bytes", sizeof(FileDropEvent));
}

TEST(mouse_move_event) {
    using namespace event;
    
    MouseMoveEvent evt{
        .position = widget::Point<int32_t>(100, 200),
        .delta = widget::Point<int32_t>(5, 10),
        .modifiers = static_cast<uint32_t>(ModifierKey::Shift),
        .timestamp = 12345
    };
    
    ASSERT_EQ(evt.position.x, 100);
    ASSERT_EQ(evt.position.y, 200);
    ASSERT_EQ(evt.delta.x, 5);
    ASSERT_EQ(evt.delta.y, 10);
    
    Event event = evt;
    ASSERT_TRUE(std::holds_alternative<MouseMoveEvent>(event));
}

TEST(mouse_button_event) {
    using namespace event;
    
    MouseButtonEvent evt{
        .button = MouseButtonEvent::Button::Left,
        .action = MouseButtonEvent::Action::Press,
        .position = widget::Point<int32_t>(50, 75),
        .modifiers = 0,
        .timestamp = 12345
    };
    
    ASSERT_EQ(evt.button, MouseButtonEvent::Button::Left);
    ASSERT_EQ(evt.action, MouseButtonEvent::Action::Press);
    ASSERT_EQ(evt.position.x, 50);
    ASSERT_EQ(evt.position.y, 75);
    
    Event event = evt;
    ASSERT_TRUE(std::holds_alternative<MouseButtonEvent>(event));
}

TEST(key_event) {
    using namespace event;
    
    KeyEvent evt{
        .keyCode = static_cast<uint32_t>(KeyCode::A),
        .action = KeyEvent::Action::Press,
        .modifiers = static_cast<uint32_t>(ModifierKey::Control),
        .timestamp = 12345
    };
    
    ASSERT_EQ(evt.keyCode, static_cast<uint32_t>(KeyCode::A));
    ASSERT_EQ(evt.action, KeyEvent::Action::Press);
    
    Event event = evt;
    ASSERT_TRUE(std::holds_alternative<KeyEvent>(event));
}

TEST(resize_event) {
    using namespace event;
    
    ResizeEvent evt{
        .newSize = widget::Size<uint32_t>(1024, 768),
        .oldSize = widget::Size<uint32_t>(800, 600)
    };
    
    ASSERT_EQ(evt.newSize.w, 1024u);
    ASSERT_EQ(evt.newSize.h, 768u);
    ASSERT_EQ(evt.oldSize.w, 800u);
    ASSERT_EQ(evt.oldSize.h, 600u);
    
    Event event = evt;
    ASSERT_TRUE(std::holds_alternative<ResizeEvent>(event));
}

TEST(file_drop_event) {
    using namespace event;
    
    std::vector<std::wstring> files = {L"file1.txt", L"file2.png"};
    FileDropEvent evt{files, widget::Point<int32_t>(100, 200)};
    
    ASSERT_TRUE(evt.payload != nullptr);
    ASSERT_EQ(evt.payload->filePaths.size(), 2);
    ASSERT_EQ(evt.payload->dropPosition.x, 100);
    ASSERT_EQ(evt.payload->dropPosition.y, 200);
    
    Event event = std::move(evt);
    ASSERT_TRUE(std::holds_alternative<FileDropEvent>(event));
}

// ============================================================================
// EVENT BUS TESTS
// ============================================================================

TEST(event_bus_subscribe) {
    using namespace event;
    
    EventBus bus;
    
    bool called = false;
    auto id = bus.subscribe([&called](const Event&) {
        called = true;
        return false;
    });
    
    ASSERT_TRUE(id > 0);
    ASSERT_EQ(bus.getListenerCount(), 1);
    
    bus.dispatch(MouseMoveEvent{});
    ASSERT_TRUE(called);
}

TEST(event_bus_unsubscribe) {
    using namespace event;
    
    EventBus bus;
    
    auto id = bus.subscribe([](const Event&) { return false; });
    ASSERT_EQ(bus.getListenerCount(), 1);
    
    bool removed = bus.unsubscribe(id);
    ASSERT_TRUE(removed);
    ASSERT_EQ(bus.getListenerCount(), 0);
}

TEST(event_bus_priority) {
    using namespace event;
    
    EventBus bus;
    
    std::vector<int> order;
    
    // Subscribe with different priorities
    bus.subscribe([&order](const Event&) {
        order.push_back(1);
        return false;
    }, 1);
    
    bus.subscribe([&order](const Event&) {
        order.push_back(3);
        return false;
    }, 3);
    
    bus.subscribe([&order](const Event&) {
        order.push_back(2);
        return false;
    }, 2);
    
    bus.dispatch(MouseMoveEvent{});
    
    // Should execute in priority order: 3, 2, 1
    ASSERT_EQ(order.size(), 3);
    ASSERT_EQ(order[0], 3);
    ASSERT_EQ(order[1], 2);
    ASSERT_EQ(order[2], 1);
}

TEST(event_bus_stop_propagation) {
    using namespace event;
    
    EventBus bus;
    
    int callCount = 0;
    
    bus.subscribe([&callCount](const Event&) {
        ++callCount;
        return true;  // Stop propagation
    }, 1);
    
    bus.subscribe([&callCount](const Event&) {
        ++callCount;
        return false;
    }, 0);
    
    bool handled = bus.dispatch(MouseMoveEvent{});
    
    ASSERT_TRUE(handled);
    ASSERT_EQ(callCount, 1);  // Only first handler should be called
}

TEST(event_bus_typed_subscription) {
    using namespace event;
    
    EventBus bus;
    
    bool mouseCalled = false;
    bool keyCalled = false;
    
    bus.subscribeType<MouseMoveEvent>([&mouseCalled](const MouseMoveEvent&) {
        mouseCalled = true;
        return false;
    });
    
    bus.subscribeType<KeyEvent>([&keyCalled](const KeyEvent&) {
        keyCalled = true;
        return false;
    });
    
    bus.dispatch(MouseMoveEvent{});
    ASSERT_TRUE(mouseCalled);
    ASSERT_TRUE(!keyCalled);
    
    mouseCalled = false;
    bus.dispatch(KeyEvent{});
    ASSERT_TRUE(!mouseCalled);
    ASSERT_TRUE(keyCalled);
}

TEST(scoped_event_listener) {
    using namespace event;
    
    EventBus bus;
    
    {
        auto listener = makeScopedListener(bus, [](const Event&) {
            return false;
        });
        
        ASSERT_EQ(bus.getListenerCount(), 1);
    }
    
    // Listener should be automatically unsubscribed
    ASSERT_EQ(bus.getListenerCount(), 0);
}

TEST(event_visitor) {
    using namespace event;
    
    Event event = MouseMoveEvent{
        .position = widget::Point<int32_t>(10, 20)
    };
    
    bool visited = false;
    int32_t x = 0, y = 0;
    
    std::visit(EventVisitor{
        [&](const MouseMoveEvent& evt) {
            visited = true;
            x = evt.position.x;
            y = evt.position.y;
        },
        [](const auto&) {}
    }, event);
    
    ASSERT_TRUE(visited);
    ASSERT_EQ(x, 10);
    ASSERT_EQ(y, 20);
}

// ============================================================================
// MODIFIER KEY TESTS
// ============================================================================

TEST(modifier_keys) {
    using namespace event;
    
    uint32_t mods = static_cast<uint32_t>(ModifierKey::Control | ModifierKey::Shift);
    
    ASSERT_TRUE(hasModifier(mods, ModifierKey::Control));
    ASSERT_TRUE(hasModifier(mods, ModifierKey::Shift));
    ASSERT_TRUE(!hasModifier(mods, ModifierKey::Alt));
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::println("=== FRQS-Widget Event System Tests ===\n");
    
    try {
        // Tests are automatically registered and run
        std::println("\n=== All tests passed! ===");
        return 0;
        
    } catch (const std::exception& e) {
        std::println(stderr, "\nTest failed with exception: {}", e.what());
        return 1;
    }
}