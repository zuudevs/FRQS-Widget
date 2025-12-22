#include "event/event_bus.hpp"
#include "event/event_types.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

// Simple assertion macro
#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        std::cerr << "Assertion failed: " << #condition << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        return 1; \
    }

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        std::cerr << "Assertion failed: " << #a << " == " << #b << " (" << (a) << " != " << (b) << ") at " << __FILE__ << ":" << __LINE__ << std::endl; \
        return 1; \
    }

using namespace frqs::event;
using namespace frqs::widget;

// ============================================================================
// TESTS
// ============================================================================

int test_basic_subscription() {
    std::cout << "Running test_basic_subscription..." << std::endl;
    
    EventBus bus;
    int callCount = 0;
    
    // Subscribe
    ListenerId id = bus.subscribe([&callCount](const Event&) {
        callCount++;
        return true;
    });
    
    ASSERT_EQ(callCount, 0);
    ASSERT_EQ(bus.getListenerCount(), 1);
    
    // Dispatch
    bus.dispatch(Event(MouseMoveEvent{Point<int32_t>(10, 10)}));
    ASSERT_EQ(callCount, 1);
    
    // Unsubscribe
    bool removed = bus.unsubscribe(id);
    ASSERT_TRUE(removed);
    ASSERT_EQ(bus.getListenerCount(), 0);
    
    // Dispatch again (should not trigger)
    bus.dispatch(Event(MouseMoveEvent{Point<int32_t>(20, 20)}));
    ASSERT_EQ(callCount, 1);
    
    std::cout << "  Passed!" << std::endl;
    return 0;
}

int test_typed_subscription() {
    std::cout << "Running test_typed_subscription..." << std::endl;
    
    EventBus bus;
    int mouseCalls = 0;
    int keyCalls = 0;
    
    // Subscribe to MouseMove
    bus.subscribeType<MouseMoveEvent>([&mouseCalls](const MouseMoveEvent& e) {
        mouseCalls++;
        return e.position.x > 0;
    });
    
    // Subscribe to KeyEvent
    bus.subscribeType<KeyEvent>([&keyCalls](const KeyEvent&) {
        keyCalls++;
        return true;
    });
    
    // Dispatch MouseMove
    bus.dispatch(Event(MouseMoveEvent{Point<int32_t>(10, 10)}));
    ASSERT_EQ(mouseCalls, 1);
    ASSERT_EQ(keyCalls, 0);
    
    // Dispatch KeyEvent
    bus.dispatch(Event(KeyEvent{KeyEvent::Action::Press, 65, 65}));
    ASSERT_EQ(mouseCalls, 1);
    ASSERT_EQ(keyCalls, 1);
    
    std::cout << "  Passed!" << std::endl;
    return 0;
}

int test_priority() {
    std::cout << "Running test_priority..." << std::endl;
    
    EventBus bus;
    std::vector<int> order;
    
    // Low priority
    auto id1 = bus.subscribe([&order](const Event&) {
        order.push_back(1);
        return false;
    }, 1);
    
    // High priority
    auto id2 = bus.subscribe([&order](const Event&) {
        order.push_back(3);
        return false;
    }, 3);
    
    // Medium priority
    auto id3 = bus.subscribe([&order](const Event&) {
        order.push_back(2);
        return false;
    }, 2);
    
    // Suppress unused warnings
    (void)id1; (void)id2; (void)id3;

    bus.dispatch(Event(WindowCloseEvent{}));
    
    ASSERT_EQ(order.size(), 3);
    ASSERT_EQ(order[0], 3); // Priority 3
    ASSERT_EQ(order[1], 2); // Priority 2
    ASSERT_EQ(order[2], 1); // Priority 1
    
    std::cout << "  Passed!" << std::endl;
    return 0;
}

int test_propagation_stop() {
    std::cout << "Running test_propagation_stop..." << std::endl;
    
    EventBus bus;
    int callCount = 0;
    
    // High priority listener that stops propagation
    auto id1 = bus.subscribe([&callCount](const Event&) {
        ++callCount;
        return true;  // Stop propagation
    }, 1);
    
    // Low priority listener (should NOT be called)
    auto id2 = bus.subscribe([&callCount](const Event&) {
        ++callCount;
        return false;
    }, 0);
    
    (void)id1; (void)id2;

    bool handled = bus.dispatch(Event(WindowCloseEvent{}));
    
    ASSERT_EQ(callCount, 1);
    ASSERT_TRUE(handled);
    
    std::cout << "  Passed!" << std::endl;
    return 0;
}

int test_scoped_listener() {
    std::cout << "Running test_scoped_listener..." << std::endl;
    
    EventBus bus;
    int mouseCalled = 0;
    int keyCalled = 0;
    
    {
        auto scoped1 = makeScopedTypedListener<MouseMoveEvent>(bus, 
            [&mouseCalled](const MouseMoveEvent&) {
                mouseCalled++;
                return true;
            }
        );
        
        auto scoped2 = makeScopedTypedListener<KeyEvent>(bus,
            [&keyCalled](const KeyEvent&) {
                keyCalled++;
                return true;
            }
        );
        
        bus.dispatch(Event(MouseMoveEvent{Point<int32_t>(0,0)}));
        bus.dispatch(Event(KeyEvent{KeyEvent::Action::Press, 0, 0}));
        
        ASSERT_EQ(mouseCalled, 1);
        ASSERT_EQ(keyCalled, 1);
    } // Listeners automatically unsubscribed here
    
    ASSERT_EQ(bus.getListenerCount(), 0);
    
    bus.dispatch(Event(MouseMoveEvent{Point<int32_t>(0,0)}));
    ASSERT_EQ(mouseCalled, 1); // Should not increase
    
    std::cout << "  Passed!" << std::endl;
    return 0;
}

// Fixed FileDropEvent Test
int test_file_drop() {
    std::cout << "Running test_file_drop..." << std::endl;

    std::vector<std::filesystem::path> files = {
        "C:\\test\\file1.txt",
        "C:\\test\\file2.jpg"
    };

    // Fix: Swap arguments (Position first, then Files)
    FileDropEvent evt{widget::Point<int32_t>(100, 200), files};

    // Fix: Access members directly (no payload->)
    ASSERT_EQ(evt.paths.size(), 2);
    ASSERT_EQ(evt.position.x, 100);
    ASSERT_EQ(evt.position.y, 200);
    
    // Verify paths content
    ASSERT_TRUE(evt.paths[0] == "C:\\test\\file1.txt");
    
    // Verify Event variant wrapping
    Event wrappedEvt = evt;
    auto* ptr = std::get_if<FileDropEvent>(&wrappedEvt);
    ASSERT_TRUE(ptr != nullptr);
    ASSERT_EQ(ptr->paths.size(), 2);

    std::cout << "  Passed!" << std::endl;
    return 0;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "EVENT SYSTEM UNIT TESTS" << std::endl;
    std::cout << "========================================" << std::endl;
    
    if (test_basic_subscription() != 0) return 1;
    if (test_typed_subscription() != 0) return 1;
    if (test_priority() != 0) return 1;
    if (test_propagation_stop() != 0) return 1;
    if (test_scoped_listener() != 0) return 1;
    if (test_file_drop() != 0) return 1;
    
    std::cout << "\nAll tests passed successfully!" << std::endl;
    return 0;
}