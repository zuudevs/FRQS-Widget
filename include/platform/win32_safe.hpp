/**
 * @file win32_safe.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Provides a safe and clean inclusion of the Windows.h header by defining common preprocessor macros 
 *        to reduce header size and undefining macros that conflict with standard library or application code.
 * @version 0.1
 * @date 2025-12-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

// Macro-safe Windows header inclusion
// Prevents pollution of user namespace

/**
 * @def WIN32_LEAN_AND_MEAN
 * @brief Excludes rarely-used services from Windows headers to speed up compilation.
 */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

/**
 * @def NOMINMAX
 * @brief Prevents Windows.h from defining min() and max() macros, which conflict with `std::min` and `std::max`.
 */
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>

#pragma comment(lib, "d2d1.lib") ///< Link against the Direct2D library for 2D graphics rendering.
#pragma comment(lib, "dwrite.lib") ///< Link against the DirectWrite library for text rendering.
#pragma comment(lib, "windowscodecs.lib") ///< Link against the Windows Imaging Component (WIC) for image processing.

/**
 * @name Windows Macro Undefinitions
 * @brief Undefine macros from Windows.h that are known to cause name collisions with common identifiers.
 * 
 * The Windows headers define many common names as macros, leading to compilation errors when they
 * conflict with function names or other symbols in C++ libraries or user code. We undefine them
 * here to create a safer environment.
 * @{
 */
#ifdef CreateWindow
#undef CreateWindow
#endif

#ifdef DrawText
#undef DrawText
#endif

#ifdef GetMessage
#undef GetMessage
#endif

#ifdef PostMessage
#undef PostMessage
#endif

#ifdef SendMessage
#undef SendMessage
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef RGB
#undef RGB
#endif

#ifdef ShellExecute
#undef ShellExecute
#endif
/** @} */

/**
 * @namespace frqs::platform
 * @brief Contains platform-specific types and functions, primarily for abstracting Windows API details.
 */
namespace frqs::platform {

/**
 * @typedef NativeHandle
 * @brief Type alias for a native window handle. On Windows, this is `HWND`.
 */
using NativeHandle = HWND;

/**
 * @typedef NativeInstance
 * @brief Type alias for a native module instance handle. On Windows, this is `HINSTANCE`.
 */
using NativeInstance = HINSTANCE;

/**
 * @typedef NativeMessage
 * @brief Type alias for a native message structure. On Windows, this is `MSG`.
 */
using NativeMessage = MSG;

/**
 * @namespace frqs::platform::unsafe
 * @brief Provides access to underlying platform details that are not guaranteed to be portable or stable.
 * @warning Use with caution. Functions in this namespace may be platform-dependent and break type safety.
 */
namespace unsafe {
    /**
     * @brief Retrieves the native window handle from a window pointer.
     * @param windowPtr A pointer to the window object.
     * @return The native window handle (`HWND`).
     * @note This is an unsafe operation that relies on implementation details of the `Window` class.
     */
    inline NativeHandle getNativeHandle(const void* windowPtr) noexcept;
} // namespace unsafe

} // namespace frqs::platform