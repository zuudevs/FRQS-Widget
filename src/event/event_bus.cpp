#include "../../include/event/event_bus.hpp"

namespace frqs::event {

// ============================================================================
// EVENT BUS IMPLEMENTATION
// ============================================================================

// Note: EventBus is mostly implemented in the header as template methods
// This file exists for any non-template implementation needs

// Example: If we need to add debugging or logging functionality
// void EventBus::debugPrint() const {
//     std::lock_guard lock(mutex_);
//     std::println("EventBus has {} listeners", listeners_.size());
//     for (const auto& listener : listeners_) {
//         std::println("  - Listener ID: {}, Priority: {}", 
//                     listener.id, listener.priority);
//     }
// }

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Create a global event bus for application-wide events
EventBus& getGlobalEventBus() {
    static EventBus globalBus;
    return globalBus;
}

} // namespace frqs::event