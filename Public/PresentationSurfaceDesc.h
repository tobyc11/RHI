#pragma once
#include <Platform.h>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

#if TC_OS == TC_OS_LINUX
#include <xcb/xcb.h>
#endif

namespace RHI
{

enum class EPresentationSurfaceDescType
{
    Win32,
    MacOS,
    Linux,
    Vulkan
};

struct CPresentationSurfaceDescWin32
{
#ifdef _WIN32
    HINSTANCE Instance;
    HWND Window;
#endif
};

struct CPresentationSurfaceDescMacOS
{
#ifdef __APPLE__
#include <Availability.h>
#ifdef __MAC_OS_X_VERSION_MAX_ALLOWED
    const void* View;
#endif
#endif
};

struct CPresentationSurfaceDescLinux
{
#if TC_OS == TC_OS_LINUX
    xcb_window_t window;
    xcb_connection_t* xconn;
#endif
};

struct CPresentationSurfaceDescVulkan
{
    // Some windowing system supports creating vulkan surface directly, this struct is provided to support such use case
    // We enable this if and only if the vulkan header is already included
    //   as we don't want presentation subsystem to force the client to depend on vulkan headers
#ifdef VK_VERSION_1_0
    VkSurfaceKHR Surface;
#endif
};

struct CPresentationSurfaceDesc
{
    EPresentationSurfaceDescType Type;
    union {
        CPresentationSurfaceDescWin32 Win32;
        CPresentationSurfaceDescMacOS MacOS;
        CPresentationSurfaceDescLinux Linux;
        CPresentationSurfaceDescVulkan Vulkan;
    };
};

} /* namespace RHI */
