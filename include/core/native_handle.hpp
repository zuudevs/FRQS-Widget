/**
 * @file native_handle.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2025-12-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <functional>
#include "platform/win32_safe.hpp"

namespace frqs::core {

// ============================================================================
// NATIVE HANDLE WRAPPER (Type-safe HWND wrapper)
// ============================================================================

/**
 * @class NativeHandle
 * @brief A lightweight, type-safe wrapper for a native platform handle (e.g., HWND).
 *
 * This class provides basic resource management for native handles. It enforces
 * unique ownership semantics by being movable but not copyable, similar to
 * `std::unique_ptr`. This helps prevent accidental duplication of handles.
 *
 * It is designed to be a zero-cost abstraction with a `constexpr` interface
 * where possible.
 */
class NativeHandle {
private:
    platform::NativeHandle handle_ = nullptr;

public:
    /**
     * @brief Default constructs a null (invalid) handle.
     */
    constexpr NativeHandle() noexcept = default;
    
    /**
     * @brief Explicitly constructs a handle from a raw native handle.
     * @param handle The platform-native handle to wrap.
     */
    explicit constexpr NativeHandle(platform::NativeHandle handle) noexcept
        : handle_(handle) {}

    // Non-copyable (handle ownership semantics)
    NativeHandle(const NativeHandle&) = delete;
    NativeHandle& operator=(const NativeHandle&) = delete;

    /**
     * @brief Move constructor. Transfers ownership from another handle.
     * @param other The NativeHandle to move from. It will be reset to null.
     */
    constexpr NativeHandle(NativeHandle&& other) noexcept
        : handle_(other.handle_) {
        other.handle_ = nullptr;
    }

    /**
     * @brief Move assignment operator. Transfers ownership from another handle.
     * @param other The NativeHandle to move from. It will be reset to null.
     * @return A reference to this object.
     */
    constexpr NativeHandle& operator=(NativeHandle&& other) noexcept {
        if (this != &other) {
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }
        return *this;
    }

    /**
     * @brief Checks if the wrapped handle is valid (not null).
     * @return `true` if the handle is not null, `false` otherwise.
     */
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return handle_ != nullptr;
    }

    /**
     * @brief Allows the handle to be used in a boolean context.
     * @return `true` if the handle is valid, `false` otherwise.
     */
    [[nodiscard]] constexpr explicit operator bool() const noexcept {
        return isValid();
    }

    /**
     * @brief Compares two handles for equality.
     * @param other The other handle to compare against.
     * @return `true` if the underlying raw handles are equal.
     */
    constexpr bool operator==(const NativeHandle& other) const noexcept {
        return handle_ == other.handle_;
    }

    /**
     * @brief Compares two handles for inequality.
     * @param other The other handle to compare against.
     * @return `true` if the underlying raw handles are not equal.
     */
    constexpr bool operator!=(const NativeHandle& other) const noexcept {
        return handle_ != other.handle_;
    }

    /**
     * @brief Resets the wrapper to a new raw handle.
     * @param handle The new handle to wrap. Defaults to null.
     */
    constexpr void reset(platform::NativeHandle handle = nullptr) noexcept {
        handle_ = handle;
    }

    /**
     * @brief Releases ownership of the raw handle.
     * @return The raw handle. The NativeHandle wrapper is reset to null.
     */
    [[nodiscard]] constexpr platform::NativeHandle release() noexcept {
        auto temp = handle_;
        handle_ = nullptr;
        return temp;
    }

    /**
     * @brief Gets the raw, unwrapped native handle.
     * @return The constant platform-native handle.
     */
    [[nodiscard]] constexpr platform::NativeHandle get() const noexcept {
        return handle_;
    }

    /**
     * @brief Gets the handle as a generic `void*`.
     * @return The handle pointer cast to `void*`.
     */
    [[nodiscard]] constexpr void* asVoidPtr() const noexcept {
        return static_cast<void*>(handle_);
    }
};

} // namespace frqs::core

// ============================================================================
// HASH SUPPORT (for use in unordered containers)
// ============================================================================

/**
 * @brief Specialization of `std::hash` for `frqs::core::NativeHandle`.
 *
 * This allows `NativeHandle` objects to be used as keys in standard unordered
 * containers like `std::unordered_map` and `std::unordered_set`.
 */
template <>
struct std::hash<frqs::core::NativeHandle> {
    /**
     * @brief Computes the hash of a NativeHandle.
     * @param handle The handle to hash.
     * @return The computed hash value.
     */
    std::size_t operator()(const frqs::core::NativeHandle& handle) const noexcept {
        // We hash the underlying pointer.
        return std::hash<void*>{}(handle.get());
    }
};