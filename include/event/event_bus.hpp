/**
 * @file event_bus.hpp
 * @brief Defines a thread-safe, priority-based event bus for pub/sub communication.
 * @author zuudevs (zuudevs@gmail.com)
 * @version 0.1
 * @date 2025-12-24
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include <functional>
#include <vector>
#include <algorithm>
#include <mutex>
#include "event.hpp"

namespace frqs::event {

// ============================================================================
// EVENT LISTENER (Type-erased callback)
// ============================================================================

/** @brief A type-erased callback function for handling any event.
 *  @param event The event to be processed.
 *  @return `true` if the event was handled and should not be propagated further
 *          (in a `dispatch` call), `false` otherwise.
 */
using EventListener = std::function<bool(const Event&)>;

/** @brief A unique identifier for a registered event listener. */
using ListenerId = uint64_t;

// ============================================================================
// EVENT BUS (Pub-Sub Pattern)
// ============================================================================

/**
 * @class EventBus
 * @brief A thread-safe, priority-based system for event publication and subscription.
 *
 * The EventBus allows different parts of an application to communicate without
 * being directly coupled. Components can subscribe to events and other components
 * can publish events. Listeners can be assigned a priority to control the order
 * of execution.
 *
 * This implementation is thread-safe for all public methods.
 */
class EventBus {
private:
    /**
     * @struct ListenerInfo
     * @brief Stores information about a single registered event listener.
     * @internal
     */
    struct ListenerInfo {
        /// @brief The unique ID of the listener.
        ListenerId id;
        /// @brief The callable function to be executed.
        EventListener callback;
        /// @brief The priority of the listener. Higher values are executed first.
        int priority;
    };

    /// @brief Mutex to protect concurrent access to the listeners list.
    mutable std::mutex mutex_;
    /// @brief The list of all registered listeners.
    std::vector<ListenerInfo> listeners_;
    /// @brief A counter to generate unique listener IDs.
    ListenerId nextId_ = 1;
    /// @brief A flag to indicate if the listeners vector needs to be re-sorted by priority.
    bool sorted_ = true;

public:
    EventBus() = default;

    // The EventBus is a central manager, so it's non-copyable and non-movable.
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;
    EventBus(EventBus&&) noexcept = delete;
    EventBus& operator=(EventBus&&) noexcept = delete;

    // ========================================================================
    // SUBSCRIBE TO EVENTS
    // ========================================================================

    /**
     * @brief Subscribes a generic callback to listen for all event types.
     * @tparam Callable The type of the callable object (e.g., lambda, function).
     * @param callback The function to call when an event is dispatched.
     * @param priority The priority of the listener. Higher values are executed first.
     * @return A unique `ListenerId` for this subscription, which can be used to unsubscribe.
     */
    template <typename Callable>
    [[nodiscard]] ListenerId subscribe(Callable&& callback, int priority = 0) {
        std::lock_guard lock(mutex_);
        
        ListenerId id = nextId_++;
        listeners_.push_back(ListenerInfo{
            .id = id,
            .callback = std::forward<Callable>(callback),
            .priority = priority
        });
        
        sorted_ = false;  // Mark for re-sorting before the next dispatch.
        return id;
    }

    /**
     * @brief Subscribes a callback to listen for a specific event type.
     *
     * This is a convenience wrapper around `subscribe`. It creates a listener
     * that checks the event type before invoking the provided callback.
     *
     * @tparam EventType The specific event type (e.g., `MouseMoveEvent`) to listen for.
     * @tparam Callable The type of the callable object.
     * @param callback The function to call, which receives the specific `EventType`.
     * @param priority The priority of the listener.
     * @return A unique `ListenerId` for this subscription.
     */
    template <typename EventType, typename Callable>
    [[nodiscard]] ListenerId subscribeType(Callable&& callback, int priority = 0) {
        return subscribe([cb = std::forward<Callable>(callback)](const Event& event) -> bool {
            if (auto* evt = std::get_if<EventType>(&event)) {
                return cb(*evt);
            }
            return false; // Event was not of the subscribed type, so not "handled".
        }, priority);
    }

    // ========================================================================
    // UNSUBSCRIBE
    // ========================================================================

    /**
     * @brief Removes a listener using its ID.
     * @param id The `ListenerId` returned by `subscribe` or `subscribeType`.
     * @return `true` if the listener was found and removed, `false` otherwise.
     */
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

    /**
     * @brief Removes all registered listeners from the event bus.
     */
    void unsubscribeAll() {
        std::lock_guard lock(mutex_);
        listeners_.clear();
    }

    // ========================================================================
    // DISPATCH EVENTS
    // ========================================================================

    /**
     * @brief Dispatches an event to all relevant listeners, in priority order.
     *
     * The dispatching stops as soon as one of the listeners returns `true`,
     * signifying that the event has been "handled".
     *
     * @param event The event to dispatch.
     * @return `true` if the event was handled by any listener, `false` otherwise.
     */
    bool dispatch(const Event& event) {
        std::lock_guard lock(mutex_);
        
        // Sort listeners by priority if a new one has been added.
        if (!sorted_) {
            std::sort(listeners_.begin(), listeners_.end(),
                [](const ListenerInfo& a, const ListenerInfo& b) {
                    return a.priority > b.priority; // Higher priority first
                });
            sorted_ = true;
        }

        // Call listeners until one handles the event.
        for (const auto& listener : listeners_) {
            if (listener.callback(event)) {
                return true;  // Event handled, stop propagation.
            }
        }
        
        return false;  // Event was not handled by anyone.
    }

    /**
     * @brief Broadcasts an event to all listeners, in priority order.
     *
     * Unlike `dispatch`, this method continues to notify all listeners even
     * if one of them returns `true`.
     *
     * @param event The event to broadcast.
     */
    void broadcast(const Event& event) {
        std::lock_guard lock(mutex_);
        
        if (!sorted_) {
            std::sort(listeners_.begin(), listeners_.end(),
                [](const ListenerInfo& a, const ListenerInfo& b) {
                    return a.priority > b.priority;
                });
            sorted_ = true;
        }

        // Call all listeners, ignoring their return value.
        for (const auto& listener : listeners_) {
            listener.callback(event);
        }
    }

    // ========================================================================
    // QUERY
    // ========================================================================

    /** @brief Gets the number of currently registered listeners. */
    [[nodiscard]] size_t getListenerCount() const noexcept {
        std::lock_guard lock(mutex_);
        return listeners_.size();
    }

    /** @brief Checks if there are any registered listeners. */
    [[nodiscard]] bool hasListeners() const noexcept {
        std::lock_guard lock(mutex_);
        return !listeners_.empty();
    }
};

// ============================================================================
// SCOPED EVENT LISTENER (RAII pattern)
// ============================================================================

/**
 * @class ScopedEventListener
 * @brief An RAII wrapper that automatically unsubscribes from an EventBus on destruction.
 *
 * This is a convenient way to manage the lifetime of a subscription, especially
 * within a class or function scope. When the ScopedEventListener object goes
 * out of scope, its destructor is called, which automatically unsubscribes
 * the listener from the event bus.
 */
class ScopedEventListener {
private:
    EventBus* bus_;
    ListenerId id_;

public:
    /**
     * @brief Constructs a ScopedEventListener.
     * @param bus The EventBus to which the listener is subscribed.
     * @param id The ID of the listener to manage.
     */
    ScopedEventListener(EventBus& bus, ListenerId id)
        : bus_(&bus), id_(id) {}

    /**
     * @brief Destructor. Unsubscribes the managed listener from the EventBus.
     */
    ~ScopedEventListener() {
        if (bus_ && id_ != 0) {
            bus_->unsubscribe(id_);
        }
    }

    // Scoped listeners manage a unique subscription, so they are non-copyable but movable.
    ScopedEventListener(const ScopedEventListener&) = delete;
    ScopedEventListener& operator=(const ScopedEventListener&) = delete;

    ScopedEventListener(ScopedEventListener&& other) noexcept
        : bus_(other.bus_), id_(other.id_) {
        other.bus_ = nullptr;
        other.id_ = 0;
    }

    ScopedEventListener& operator=(ScopedEventListener&& other) noexcept {
        if (this != &other) {
            if (bus_ && id_ != 0) bus_->unsubscribe(id_);
            bus_ = other.bus_;
            id_ = other.id_;
            other.bus_ = nullptr;
            other.id_ = 0;
        }
        return *this;
    }

    /** @brief Gets the ID of the managed listener. */
    [[nodiscard]] ListenerId getId() const noexcept { return id_; }
};

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * @brief A helper function to create a `ScopedEventListener`.
 * @tparam Callable The type of the callback function.
 * @param bus The EventBus to subscribe to.
 * @param callback The callback function.
 * @param priority The priority of the listener.
 * @return A `ScopedEventListener` that will automatically unsubscribe on destruction.
 */
template <typename Callable>
[[nodiscard]] inline ScopedEventListener makeScopedListener(
    EventBus& bus,
    Callable&& callback,
    int priority = 0
) {
    auto id = bus.subscribe(std::forward<Callable>(callback), priority);
    return ScopedEventListener(bus, id);
}

/**
 * @brief A helper function to create a typed `ScopedEventListener`.
 * @tparam EventType The specific event type to listen for.
 * @tparam Callable The type of the callback function.
 * @param bus The EventBus to subscribe to.
 * @param callback The callback function that accepts `EventType`.
 * @param priority The priority of the listener.
 * @return A `ScopedEventListener` that will automatically unsubscribe on destruction.
 */
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