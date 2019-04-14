#pragma once
#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

namespace RHI
{

enum class EPresentationSurfaceDescType
{
    Win32,
    MacOS
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

struct CPresentationSurfaceDesc
{
    EPresentationSurfaceDescType Type;
    union {
        CPresentationSurfaceDescWin32 Win32;
        CPresentationSurfaceDescMacOS MacOS;
    };
};

} /* namespace RHI */
