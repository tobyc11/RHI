#pragma once
#include <Platform.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#if TC_OS == TC_OS_MAC_OS_X
#define VK_USE_PLATFORM_MACOS_MVK
#endif

#include "vk_mem_alloc.h"

namespace RHI
{

class CDeviceVk;
class CCommandContextVk;
class CImageVk;
class CImageViewVk;

}
