#pragma once

// Macro-safe Windows header inclusion
// Prevents pollution of user namespace

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <windowsx.h>
#include <d2d1.h>
#include <dwrite.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

// Undefine common conflicting macros immediately after inclusion
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

namespace frqs::platform {

// Safe type aliases for Windows types
using NativeHandle = HWND;
using NativeInstance = HINSTANCE;
using NativeMessage = MSG;

// Unsafe namespace for advanced users only
namespace unsafe {
    inline NativeHandle getNativeHandle(const void* windowPtr) noexcept;
} // namespace unsafe

} // namespace frqs::platform