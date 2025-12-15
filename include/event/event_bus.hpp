#pragma once

#include <functional>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include "event.hpp"

namespace frqs::event {

// ============================================================================
// EVENT LISTENER (Type-erased callback)
// ============================================================================

using EventListener = std::function<bool(const Event&)>;
using ListenerId = uint64_t;

// ============================================================================
// EVENT BUS (Pub-Sub Pattern)
// ============================================================================

class EventBus {
private:
    struct ListenerInfo {
        ListenerId id;
        EventListener callback;
        int priority;  // Higher = earlier execution
    };

    mutable std::mutex mutex_;
    std::vector<ListenerInfo> listeners_;
    ListenerId nextId_ = 1;
    bool sorted_ = true;

public:
    EventBus() = default;

    // Non-copyable, movable
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;
    EventBus(EventBus&&) noexcept = default;
    EventBus& operator=(EventBus&&) noexcept = default;

    // ========================================================================
    // SUBSCRIBE TO EVENTS
    // ========================================================================

    // Subscribe with any callable (lambda, function pointer, etc.)
    template <typename Callable>
    [[nodiscard]] ListenerId subscribe(Callable&& callback, int priority = 0) {
        std::lock_guard lock(mutex_);
        
        ListenerId id = nextId_++;
        listeners_.push_back(ListenerInfo{
            .id = id,
            .callback = std::forward<Callable>(callback),
            .priority = priority
        });
        
        sorted_ = false;  // Mark for re-sorting
        return id;
    }

    // Subscribe to specific event type with visitor pattern
    template <typename EventType, typename Callable>
    [[nodiscard]] ListenerId subscribeType(Callable&& callback, int priority = 0) {
        return subscribe([cb = std::forward<Callable>(callback)](const Event& event) -> bool {
            if (auto* evt = std::get_if<EventType>(&event)) {
                return cb(*evt);
            }
            return false;  // Event not handled
        }, priority);
    }

    // ========================================================================
    // UNSUBSCRIBE
    // ========================================================================

    bool unsubscribe(ListenerId id) {
        std::lock_guard lock(mutex_);
        
        auto it = std::find_if(listeners_.begin(), listeners_.end(),
            [id](const ListenerInfo& info) { return info.id == id; });
        
        if (it != listeners_.end()) {
            listeners_.erase(it);
            return true;
        }
        return false;
    }

    void unsubscribeAll() {
        std::lock_guard lock(mutex_);
        listeners_.clear();
    }

    // ========================================================================
    // DISPATCH EVENTS
    // ========================================================================

    // Dispatch event to all listeners (stops if any returns true)
    bool dispatch(const Event& event) {
        std::lock_guard lock(mutex_);
        
        // Sort by priority if needed
        if (!sorted_) {
            std::sort(listeners_.begin(), listeners_.end(),
                [](const ListenerInfo& a, const ListenerInfo& b) {
                    return a.priority > b.priority;  // Higher priority first
                });
            sorted_ = true;
        }

        // Call listeners until one handles the event
        for (auto& listener : listeners_) {
            if (listener.callback(event)) {
                return true;  // Event handled
            }
        }
        
        return false;  // Event not handled
    }

    // Dispatch to all listeners (doesn't stop on true)
    void broadcast(const Event& event) {
        std::lock_guard lock(mutex_);
        
        if (!sorted_) {
            std::sort(listeners_.begin(), listeners_.end(),
                [](const ListenerInfo& a, const ListenerInfo& b) {
                    return a.priority > b.priority;
                });
            sorted_ = true;
        }

        for (auto& listener : listeners_) {
            listener.callback(event);  // Ignore return value
        }
    }

    // ========================================================================
    // QUERY
    // ========================================================================

    [[nodiscard]] size_t getListenerCount() const noexcept {
        std::lock_guard lock(mutex_);
        return listeners_.size();
    }

    [[nodiscard]] bool hasListeners() const noexcept {
        std::lock_guard lock(mutex_);
        return !listeners_.empty();
    }
};

// ============================================================================
// SCOPED EVENT LISTENER (RAII pattern)
// ============================================================================

class ScopedEventListener {
private:
    EventBus* bus_;
    ListenerId id_;

public:
    ScopedEventListener(EventBus& bus, ListenerId id)
        : bus_(&bus), id_(id) {}

    ~ScopedEventListener() {
        if (bus_) {
            bus_->unsubscribe(id_);
        }
    }

    // Non-copyable, movable
    ScopedEventListener(const ScopedEventListener&) = delete;
    ScopedEventListener& operator=(const ScopedEventListener&) = delete;

    ScopedEventListener(ScopedEventListener&& other) noexcept
        : bus_(other.bus_), id_(other.id_) {
        other.bus_ = nullptr;
    }

    ScopedEventListener& operator=(ScopedEventListener&& other) noexcept {
        if (this != &other) {
            if (bus_) bus_->unsubscribe(id_);
            bus_ = other.bus_;
            id_ = other.id_;
            other.bus_ = nullptr;
        }
        return *this;
    }

    [[nodiscard]] ListenerId getId() const noexcept { return id_; }
};

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Create scoped listener
template <typename Callable>
[[nodiscard]] inline ScopedEventListener makeScopedListener(
    EventBus& bus, 
    Callable&& callback, 
    int priority = 0
) {
    auto id = bus.subscribe(std::forward<Callable>(callback), priority);
    return ScopedEventListener(bus, id);
}

// Create scoped typed listener
template <typename EventType, typename Callable>
[[nodiscard]] inline ScopedEventListener makeScopedTypedListener(
    EventBus& bus,
    Callable&& callback,
    int priority = 0
) {
    auto id = bus.subscribeType<EventType>(std::forward<Callable>(callback), priority);
    return ScopedEventListener(bus, id);
}

} // namespace frqs::event