#include "ImageVk.h"
#include "CommandContextVk.h"
#include "DeviceVk.h"

namespace RHI
{

CImageVk::CImageVk(CDeviceVk& p, VkImage image, VmaAllocation alloc,
                   const VkImageCreateInfo& createInfo, EImageUsageFlags usage,
                   EResourceState defaultState)
    : Parent(p)
    , Image(image)
    , ImageAlloc(alloc)
    , CreateInfo(createInfo)
    , UsageFlags(usage)
    , DefaultState(defaultState)
{
    GlobalState = DefaultState;
}

CImageVk::~CImageVk() { vmaDestroyImage(Parent.GetAllocator(), Image, ImageAlloc); }

void CImageVk::CopyFrom(const void* mem) { throw "unimplemented"; }

VkImageCreateInfo CImageVk::GetCreateInfo() const { return CreateInfo; }

EImageUsageFlags CImageVk::GetUsageFlags() const { return UsageFlags; }

EResourceState CImageVk::GetDefaultState() const { return DefaultState; }

EResourceState CImageVk::GetGlobalState() const { return GlobalState; }

void CImageVk::SetGlobalState(EResourceState state) { GlobalState = state; }

}
