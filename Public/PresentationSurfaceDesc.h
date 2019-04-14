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

#ifdef _WIN32
struct CPresentationSurfaceDescWin32
{
    HINSTANCE Instance;
    HWND Window;
};
#endif

struct CPresentationSurfaceDesc
{
    EPresentationSurfaceDescType Type;
    union {
#ifdef _WIN32
        CPresentationSurfaceDescWin32 Win32;
#endif
    };
};

} /* namespace RHI */
