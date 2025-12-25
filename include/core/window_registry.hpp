/**
 * @file window_registry.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2025-12-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <unordered_map>
#include <vector>
#include <mutex>
#include <memory>
#include <optional>
#include <algorithm>
#include "window_id.hpp"

namespace frqs::core {

class Window; // Forward declaration

// ============================================================================
// WINDOW REGISTRY (Singleton, Thread-Safe)
// ============================================================================

/**
 * @class WindowRegistry
 * @brief A thread-safe singleton for managing all active windows in the application.
 *
 * This class provides a central repository for all `Window` instances. It allows
 * for thread-safe registration, unregistration, and lookup of windows by their
 * unique `WindowId`. It also maintains the z-order of windows, which can be
 * used for rendering or activation logic.
 */
class WindowRegistry {
public:
    /// @brief A shared pointer to a Window object.
    using WindowPtr = std::shared_ptr<Window>;

private:
    /// @brief Mutex to ensure thread-safe access to the registry's data.
    mutable std::mutex mutex_;
    /// @brief Map for fast, O(1) average time complexity lookup of windows by ID.
    std::unordered_map<WindowId, WindowPtr> windows_;
    /// @brief Vector maintaining the Z-order of windows, from top-most (front) to bottom-most (back).
    std::vector<WindowId> zOrder_;
    /// @brief A simple counter to generate unique window IDs.
    uint64_t nextId_ = 1;

    WindowRegistry() = default;

public:
    /**
     * @brief Gets the singleton instance of the WindowRegistry.
     * @return A reference to the single WindowRegistry instance.
     */
    static WindowRegistry& instance() noexcept {
        static WindowRegistry registry;
        return registry;
    }

    // The registry is a singleton, so it cannot be copied or moved.
    WindowRegistry(const WindowRegistry&) = delete;
    WindowRegistry(WindowRegistry&&) = delete;
    WindowRegistry& operator=(const WindowRegistry&) = delete;
    WindowRegistry& operator=(WindowRegistry&&) = delete;

    /**
     * @brief Registers a new window, assigns it an ID, and adds it to the registry.
     * @param window A `shared_ptr` to the window to register.
     * @return The newly assigned, unique `WindowId` for the window.
     */
    [[nodiscard]] WindowId registerWindow(WindowPtr window) {
        std::lock_guard lock(mutex_);
        WindowId id{nextId_++};
        windows_[id] = std::move(window);
        zOrder_.insert(zOrder_.begin(), id);  // Add to top of Z-order (front)
        return id;
    }

    /**
     * @brief Removes a window from the registry using its ID.
     * @param id The `WindowId` of the window to unregister.
     * @return `true` if the window was found and removed, `false` otherwise.
     */
    bool unregisterWindow(WindowId id) noexcept {
        std::lock_guard lock(mutex_);
        
        // Remove from the main map
        if (windows_.erase(id) == 0) {
            return false; // Window with that ID did not exist
        }
        
        // Remove from Z-order vector
        auto it = std::find(zOrder_.begin(), zOrder_.end(), id);
        if (it != zOrder_.end()) {
            zOrder_.erase(it);
        }
        
        return true;
    }

    /**
     * @brief Retrieves a window by its ID.
     * @param id The `WindowId` of the window to find.
     * @return An `std::optional<WindowPtr>` containing the window's `shared_ptr`
     *         if found, or `std::nullopt` if not.
     */
    [[nodiscard]] std::optional<WindowPtr> getWindow(WindowId id) const noexcept {
        std::lock_guard lock(mutex_);
        auto it = windows_.find(id);
        return (it != windows_.end()) ? std::optional{it->second} : std::nullopt;
    }

    /**
     * @brief Moves a window to the front of the Z-order.
     * @param id The `WindowId` of the window to bring to the front.
     */
    void bringToFront(WindowId id) noexcept {
        std::lock_guard lock(mutex_);
        
        auto it = std::find(zOrder_.begin(), zOrder_.end(), id);
        // If found and not already at the front
        if (it != zOrder_.end() && it != zOrder_.begin()) {
            std::rotate(zOrder_.begin(), it, it + 1); // Efficiently move element to front
        }
    }

    /**
     * @brief Retrieves all registered windows, sorted by their Z-order (top-most first).
     * @return A `std::vector` of `WindowPtr`s.
     */
    [[nodiscard]] std::vector<WindowPtr> getAllWindows() const {
        std::lock_guard lock(mutex_);
        std::vector<WindowPtr> result;
        result.reserve(zOrder_.size());
        
        // Iterate through Z-order and look up the corresponding window pointer
        for (const auto& id : zOrder_) {
            if (auto it = windows_.find(id); it != windows_.end()) {
                result.push_back(it->second);
            }
        }
        
        return result;
    }

    /**
     * @brief Gets the current number of registered windows.
     * @return The number of windows in the registry.
     */
    [[nodiscard]] size_t getWindowCount() const noexcept {
        std::lock_guard lock(mutex_);
        return windows_.size();
    }

    /**
     * @brief Removes all windows from the registry.
     * @note This will likely lead to the destruction of the window objects if
     *       no other `shared_ptr`s to them exist.
     */
    void clear() noexcept {
        std::lock_guard lock(mutex_);
        // When the map is cleared, the shared_ptr reference count will decrease,
        // potentially destroying the window objects.
        windows_.clear();
        zOrder_.clear();
    }
};

} // namespace frqs::core