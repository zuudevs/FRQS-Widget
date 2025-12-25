/**
 * @file event.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines the core `Event` type and related utilities for the event system.
 * @version 0.1
 * @date 2025-12-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <variant>
#include "event_types.hpp"

namespace frqs::event {

// ============================================================================
// HYBRID VARIANT EVENT TYPE
// ============================================================================

/**
 * @brief A variant type representing any possible event in the system.
 *
 * `Event` uses `std::variant` to store different event structures in a
 * type-safe and memory-efficient way. This approach is a hybrid, balancing
 * performance for frequently occurring ("hot") events and flexibility for
 * less frequent ("cold") events.
 *
 * @see MouseMoveEvent, MouseButtonEvent, KeyEvent, ResizeEvent, PaintEvent, FileDropEvent, MouseWheelEvent
 */
using Event = std::variant<
    std::monostate,        ///< Represents an empty or null event.
    MouseMoveEvent,        ///< Event for mouse movement. (Hot path)
    MouseButtonEvent,      ///< Event for mouse button presses/releases. (Hot path)
    KeyEvent,              ///< Event for keyboard key presses/releases. (Hot path)
    ResizeEvent,           ///< Event for window resizing. (Hot path)
    PaintEvent,            ///< Event signaling a need to repaint a region. (Hot path)
    FileDropEvent,         ///< Event for files dropped onto a window. (Cold path, variable size)
    MouseWheelEvent        ///< Event for mouse wheel scrolling. (Hot path)
>;

// Static assertion to ensure the Event variant's size stays within a reasonable
// limit for performance-critical paths.
static_assert(sizeof(Event) <= 128, "Event variant too large for hot path");

// ============================================================================
// EVENT VISITOR HELPERS
// ============================================================================

/**
 * @brief A visitor for handling `frqs::event::Event` variants.
 *
 * This struct leverages the "overloaded" pattern using variadic templates and
 * inheritance. It allows constructing a callable object from a set of lambdas,
 * each handling a specific event type from the `Event` variant.
 *
 * @tparam Handlers A pack of callable types (e.g., lambdas), each taking one of
 *                  the event types contained in `frqs::event::Event` as an argument.
 *
 * @example
 * std::visit(EventVisitor{
 *   [](const MouseMoveEvent& e) { std::cout << "Mouse moved\n"; },
 *   [](const KeyEvent& e) { std::cout << "Key pressed\n"; },
 *   [](auto) { } // Generic fallback for other event types
 * }, event_instance);
 */
template <typename... Handlers>
struct EventVisitor : Handlers... {
    using Handlers::operator()...;
};

/**
 * @brief Deduction guide for `EventVisitor`.
 *
 * Allows for creating an `EventVisitor` instance without explicitly specifying
 * the template arguments.
 *
 * @tparam Handlers The types of the provided handler lambdas.
 */
template <typename... Handlers>
EventVisitor(Handlers...) -> EventVisitor<Handlers...>;

// ============================================================================
// EVENT TYPE CHECKS
// ============================================================================

/**
 * @brief A concept that checks if a type `T` is a valid event type within the system.
 *
 * This concept ensures that a given type is one of the types included in the
 * `frqs::event::Event` variant, providing compile-time validation for template
 * functions or classes that operate on specific event types.
 *
 * @tparam T The type to check.
 */
template <typename T>
concept EventType = std::is_same_v<T, MouseMoveEvent> ||
                    std::is_same_v<T, MouseButtonEvent> ||
                    std::is_same_v<T, KeyEvent> ||
                    std::is_same_v<T, ResizeEvent> ||
                    std::is_same_v<T, PaintEvent> ||
                    std::is_same_v<T, FileDropEvent> ||
                    std::is_same_v<T, MouseWheelEvent>;

} // namespace frqs::event