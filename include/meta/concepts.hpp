/**
 * @file concepts.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines a set of C++20 concepts for type-checking and compile-time validation across the FRQS Widget library.
 * @version 0.1
 * @date 2025-12-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <iterator>
#include <type_traits>
#include <concepts>

namespace frqs::meta {

// ============================================================================
// BASIC TYPE CONCEPTS
// ============================================================================

/**
 * @concept numeric
 * @brief Checks if a type `T` is an arithmetic type (either integral or floating-point).
 */
template <typename T>
concept numeric = std::is_arithmetic_v<T>;

/**
 * @concept pointer_like
 * @brief Checks if a type `T` behaves like a pointer, including raw pointers and smart pointers.
 *
 * A type is considered pointer-like if it is a raw pointer or if it has `operator*` and a `get()` method
 * that returns a raw pointer.
 */
template <typename T>
concept pointer_like = std::is_pointer_v<T> || requires(T t) {
    { *t } -> std::convertible_to<typename T::element_type&>;
    { t.get() } -> std::convertible_to<typename T::element_type*>;
};

// ============================================================================
// WIDGET TYPE CONCEPTS
// ============================================================================

// Forward declarations for widget checking
namespace widget {
    class IWidget;
    class Widget;
}

/**
 * @concept widget_type
 * @brief Checks if a type `T` is derived from `frqs::widget::IWidget`.
 *
 * This concept is used to constrain templates to accept only types that are part of the widget hierarchy.
 */
template <typename T>
concept widget_type = std::is_base_of_v<widget::IWidget, T> || 
                     std::is_same_v<T, widget::IWidget>;

/**
 * @concept concrete_widget
 * @brief Checks if a type `T` is a concrete widget, i.e., derived from `frqs::widget::Widget`.
 */
template <typename T>
concept concrete_widget = std::is_base_of_v<widget::Widget, T> ||
                         std::is_same_v<T, widget::Widget>;

// ============================================================================
// EVENT HANDLER CONCEPTS
// ============================================================================

namespace event {
    struct Event;
}

/**
 * @concept event_handler
 * @brief Checks if a type `F` is a valid event handler for a generic `frqs::event::Event`.
 *
 * An event handler must be invocable with a `const frqs::event::Event&` and return a boolean value.
 */
template <typename F>
concept event_handler = std::invocable<F, const event::Event&> &&
                       std::convertible_to<std::invoke_result_t<F, const event::Event&>, bool>;

/**
 * @concept typed_event_handler
 * @brief Checks if a type `F` is a valid event handler for a specific event type `EventType`.
 *
 * The handler must be invocable with `const EventType&` and return a boolean value.
 */
template <typename F, typename EventType>
concept typed_event_handler = std::invocable<F, const EventType&> &&
                              std::convertible_to<std::invoke_result_t<F, const EventType&>, bool>;

// ============================================================================
// RENDER CONCEPTS
// ============================================================================

namespace widget {
    class Renderer;
}

/**
 * @concept renderable
 * @brief Checks if a type `T` can be rendered.
 *
 * A type is renderable if it has a `render` method that takes a `frqs::widget::Renderer&` and returns `void`.
 */
template <typename T>
concept renderable = requires(T t, widget::Renderer& renderer) {
    { t.render(renderer) } -> std::same_as<void>;
};

/**
 * @concept has_rect
 * @brief Checks if a type `T` has `getRect` and `setRect` methods for manipulating its bounding box.
 */
template <typename T>
concept has_rect = requires(T t) {
    { t.getRect() };
    { t.setRect(typename std::remove_cvref_t<T>::rect_type{}) };
};

// ============================================================================
// CALLABLE CONCEPTS
// ============================================================================

/**
 * @concept ui_task
 * @brief Checks if a type `F` represents a task to be run on the UI thread.
 *
 * A UI task must be invocable with no arguments and return `void`.
 */
template <typename F>
concept ui_task = std::invocable<F> && std::same_as<std::invoke_result_t<F>, void>;

/**
 * @concept callback
 * @brief A general-purpose concept for any invocable type `F` with the given arguments.
 */
template <typename F, typename... Args>
concept callback = std::invocable<F, Args...>;

// ============================================================================
// CONTAINER CONCEPTS
// ============================================================================

/**
 * @concept iterable
 * @brief Checks if a type `T` is iterable using `begin()` and `end()`.
 */
template <typename T>
concept iterable = requires(T t) {
    { t.begin() } -> std::input_or_output_iterator;
    { t.end() } -> std::input_or_output_iterator;
};

/**
 * @concept container
 * @brief Checks if a type `T` behaves like a standard container.
 *
 * A container must be iterable and have `size()` and `empty()` methods, as well as a `value_type` member type.
 */
template <typename T>
concept container = iterable<T> && requires(T t) {
    typename T::value_type;
    { t.size() } -> std::convertible_to<std::size_t>;
    { t.empty() } -> std::convertible_to<bool>;
};

// ============================================================================
// MEMORY CONCEPTS
// ============================================================================

/**
 * @concept smart_pointer
 * @brief Checks if a type `T` conforms to the interface of a smart pointer.
 *
 * A smart pointer must have `get()`, `operator->()`, `operator*()`, and an `element_type` member type.
 */
template <typename T>
concept smart_pointer = requires(T ptr) {
    typename T::element_type;
    { ptr.get() } -> std::convertible_to<typename T::element_type*>;
    { ptr.operator->() } -> std::convertible_to<typename T::element_type*>;
    { ptr.operator*() } -> std::convertible_to<typename T::element_type&>;
};

/**
 * @concept unique_ptr_like
 * @brief Checks if a type `T` behaves like a `std::unique_ptr` (movable, not copyable).
 */
template <typename T>
concept unique_ptr_like = smart_pointer<T> && !std::is_copy_constructible_v<T>;

/**
 * @concept shared_ptr_like
 * @brief Checks if a type `T` behaves like a `std::shared_ptr` (copyable).
 */
template <typename T>
concept shared_ptr_like = smart_pointer<T> && std::is_copy_constructible_v<T>;

} // namespace frqs::meta