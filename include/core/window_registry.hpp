#pragma once

#include <unordered_map>
#include <vector>
#include <mutex>
#include <memory>
#include <optional>
#include <cstdint>

namespace frqs::core {

class Window;

// ============================================================================
// WINDOW ID TYPE (Strong Type for Safety)
// ============================================================================

struct WindowId {
    uint64_t value;
    
    constexpr bool operator==(const WindowId&) const noexcept = default;
    constexpr auto operator<=>(const WindowId&) const noexcept = default;
};

} // namespace frqs::core

// Hash specialization for WindowId
template <>
struct std::hash<frqs::core::WindowId> {
    std::size_t operator()(const frqs::core::WindowId& id) const noexcept {
        return std::hash<uint64_t>{}(id.value);
    }
};

namespace frqs::core {

// ============================================================================
// WINDOW REGISTRY (Singleton, Thread-Safe)
// ============================================================================

class WindowRegistry {
public:
    using WindowPtr = std::shared_ptr<Window>;

private:
    mutable std::mutex mutex_;
    std::unordered_map<WindowId, WindowPtr> windows_;  // O(1) lookup
    std::vector<WindowId> zOrder_;                     // Z-order (front to back)
    uint64_t nextId_ = 1;

    WindowRegistry() = default;

public:
    // Singleton access
    static WindowRegistry& instance() noexcept {
        static WindowRegistry registry;
        return registry;
    }

    // Non-copyable, non-movable
    WindowRegistry(const WindowRegistry&) = delete;
    WindowRegistry(WindowRegistry&&) = delete;
    WindowRegistry& operator=(const WindowRegistry&) = delete;
    WindowRegistry& operator=(WindowRegistry&&) = delete;

    // Register a new window
    [[nodiscard]] WindowId registerWindow(WindowPtr window) {
        std::lock_guard lock(mutex_);
        WindowId id{nextId_++};
        windows_[id] = std::move(window);
        zOrder_.push_back(id);  // Add to top of Z-order
        return id;
    }

    // Unregister a window
    bool unregisterWindow(WindowId id) noexcept {
        std::lock_guard lock(mutex_);
        
        auto it = windows_.find(id);
        if (it == windows_.end()) return false;
        
        windows_.erase(it);
        
        // Remove from Z-order
        auto zit = std::find(zOrder_.begin(), zOrder_.end(), id);
        if (zit != zOrder_.end()) {
            zOrder_.erase(zit);
        }
        
        return true;
    }

    // Get window by ID
    [[nodiscard]] std::optional<WindowPtr> getWindow(WindowId id) const noexcept {
        std::lock_guard lock(mutex_);
        auto it = windows_.find(id);
        return (it != windows_.end()) ? std::optional{it->second} : std::nullopt;
    }

    // Bring window to front
    void bringToFront(WindowId id) noexcept {
        std::lock_guard lock(mutex_);
        
        auto it = std::find(zOrder_.begin(), zOrder_.end(), id);
        if (it != zOrder_.end()) {
            zOrder_.erase(it);
            zOrder_.insert(zOrder_.begin(), id);  // Move to front
        }
    }

    // Get all windows in Z-order
    [[nodiscard]] std::vector<WindowPtr> getAllWindows() const {
        std::lock_guard lock(mutex_);
        std::vector<WindowPtr> result;
        result.reserve(zOrder_.size());
        
        for (const auto& id : zOrder_) {
            if (auto it = windows_.find(id); it != windows_.end()) {
                result.push_back(it->second);
            }
        }
        
        return result;
    }

    // Get window count
    [[nodiscard]] size_t getWindowCount() const noexcept {
        std::lock_guard lock(mutex_);
        return windows_.size();
    }

    // Clear all windows
    void clear() noexcept {
        std::lock_guard lock(mutex_);
        windows_.clear();
        zOrder_.clear();
    }
};

} // namespace frqs::core