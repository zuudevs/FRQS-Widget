#include "../../include/core/window_registry.hpp"

namespace frqs::core {

// ============================================================================
// WINDOW REGISTRY IMPLEMENTATION
// ============================================================================

// Note: WindowRegistry is implemented entirely in the header as inline methods
// This file exists for potential future non-inline implementations

// Example: If we need to add logging or debugging functionality
// void WindowRegistry::debugPrintRegistry() const {
//     std::lock_guard lock(mutex_);
//     for (const auto& [id, window] : windows_) {
//         // Print window info
//     }
// }

} // namespace frqs::core