/**
 * @file event_bus.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Implementation file for the EventBus and related helpers.
 * @version 0.1
 * @date 2025-12-24
 *
 * @copyright Copyright (c) 2025
 *
 * This file provides the implementation for any non-templated methods of the
 * EventBus class and defines the global event bus instance accessor. Most of
 * the EventBus logic resides in the header file (`event_bus.hpp`) due to its
 * heavy use of templates.
 */

#include "event/event_bus.hpp"

namespace frqs::event {

// ============================================================================
// EVENT BUS IMPLEMENTATION
// ============================================================================

// Most of the EventBus is implemented in the header via template methods.
// This file is the location for any future non-templated method definitions.

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Retrieves the global, application-wide EventBus instance.
 *
 * This function provides a singleton `EventBus` that can be used for system-wide
 * event communication, decoupling major components of the application.
 *
 * @return A reference to the global EventBus singleton.
 */
EventBus& getGlobalEventBus() {
    static EventBus globalBus;
    return globalBus;
}

} // namespace frqs::event