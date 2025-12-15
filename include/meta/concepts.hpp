#pragma once

#include <iterator>
#include <type_traits>
#include <concepts>

namespace frqs::meta {

// ============================================================================
// BASIC TYPE CONCEPTS
// ============================================================================

template <typename T>
concept numeric = std::is_arithmetic_v<T>;

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

template <typename T>
concept widget_type = std::is_base_of_v<widget::IWidget, T> || 
                     std::is_same_v<T, widget::IWidget>;

template <typename T>
concept concrete_widget = std::is_base_of_v<widget::Widget, T> ||
                         std::is_same_v<T, widget::Widget>;

// ============================================================================
// EVENT HANDLER CONCEPTS
// ============================================================================

namespace event {
    struct Event;
}

template <typename F>
concept event_handler = std::invocable<F, const event::Event&> &&
                       std::convertible_to<std::invoke_result_t<F, const event::Event&>, bool>;

template <typename F, typename EventType>
concept typed_event_handler = std::invocable<F, const EventType&> &&
                              std::convertible_to<std::invoke_result_t<F, const EventType&>, bool>;

// ============================================================================
// RENDER CONCEPTS
// ============================================================================

namespace widget {
    class Renderer;
}

template <typename T>
concept renderable = requires(T t, widget::Renderer& renderer) {
    { t.render(renderer) } -> std::same_as<void>;
};

template <typename T>
concept has_rect = requires(T t) {
    { t.getRect() };
    { t.setRect(typename std::remove_cvref_t<T>::rect_type{}) };
};

// ============================================================================
// CALLABLE CONCEPTS
// ============================================================================

template <typename F>
concept ui_task = std::invocable<F> && std::same_as<std::invoke_result_t<F>, void>;

template <typename F, typename... Args>
concept callback = std::invocable<F, Args...>;

// ============================================================================
// CONTAINER CONCEPTS
// ============================================================================

template <typename T>
concept iterable = requires(T t) {
    { t.begin() } -> std::input_or_output_iterator;
    { t.end() } -> std::input_or_output_iterator;
};

template <typename T>
concept container = iterable<T> && requires(T t) {
    typename T::value_type;
    { t.size() } -> std::convertible_to<std::size_t>;
    { t.empty() } -> std::convertible_to<bool>;
};

// ============================================================================
// MEMORY CONCEPTS
// ============================================================================

template <typename T>
concept smart_pointer = requires(T ptr) {
    typename T::element_type;
    { ptr.get() } -> std::convertible_to<typename T::element_type*>;
    { ptr.operator->() } -> std::convertible_to<typename T::element_type*>;
    { ptr.operator*() } -> std::convertible_to<typename T::element_type&>;
};

template <typename T>
concept unique_ptr_like = smart_pointer<T> && !std::is_copy_constructible_v<T>;

template <typename T>
concept shared_ptr_like = smart_pointer<T> && std::is_copy_constructible_v<T>;

} // namespace frqs::meta