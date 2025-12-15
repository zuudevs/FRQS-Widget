#pragma once

#include <functional>
#include "../platform/win32_safe.hpp"

namespace frqs::core {

// ============================================================================
// NATIVE HANDLE WRAPPER (Type-safe HWND wrapper)
// ============================================================================

class NativeHandle {
private:
    platform::NativeHandle handle_ = nullptr;

public:
    constexpr NativeHandle() noexcept = default;
    
    explicit constexpr NativeHandle(platform::NativeHandle handle) noexcept
        : handle_(handle) {}

    // Non-copyable (handle ownership semantics)
    NativeHandle(const NativeHandle&) = delete;
    NativeHandle& operator=(const NativeHandle&) = delete;

    // Movable
    constexpr NativeHandle(NativeHandle&& other) noexcept
        : handle_(other.handle_) {
        other.handle_ = nullptr;
    }

    constexpr NativeHandle& operator=(NativeHandle&& other) noexcept {
        if (this != &other) {
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }
        return *this;
    }

    // Check validity
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return handle_ != nullptr;
    }

    [[nodiscard]] constexpr explicit operator bool() const noexcept {
        return isValid();
    }

    // Comparison
    constexpr bool operator==(const NativeHandle& other) const noexcept {
        return handle_ == other.handle_;
    }

    constexpr bool operator!=(const NativeHandle& other) const noexcept {
        return handle_ != other.handle_;
    }

    // Reset handle
    constexpr void reset(platform::NativeHandle handle = nullptr) noexcept {
        handle_ = handle;
    }

    // Release ownership (returns raw handle and resets to nullptr)
    [[nodiscard]] constexpr platform::NativeHandle release() noexcept {
        auto temp = handle_;
        handle_ = nullptr;
        return temp;
    }

    // Get raw handle (const only - prevents modification)
    [[nodiscard]] constexpr platform::NativeHandle get() const noexcept {
        return handle_;
    }

    // Convert to void* for generic APIs
    [[nodiscard]] constexpr void* asVoidPtr() const noexcept {
        return static_cast<void*>(handle_);
    }
};

} // namespace frqs::core

// ============================================================================
// HASH SUPPORT (for use in unordered containers)
// ============================================================================

template <>
struct std::hash<frqs::core::NativeHandle> {
    std::size_t operator()(const frqs::core::NativeHandle& handle) const noexcept {
        return std::hash<void*>{}(handle.get());
    }
};