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
    Linux
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

struct CPresentationSurfaceDesc
{
    EPresentationSurfaceDescType Type;
    union {
        CPresentationSurfaceDescWin32 Win32;
        CPresentationSurfaceDescMacOS MacOS;
        CPresentationSurfaceDescLinux Linux;
    };
};

} /* namespace RHI */
