#include "ImageVk.h"
#include "CommandContextVk.h"
#include "DeviceVk.h"
#include "SwapChainVk.h"

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
    GlobalState = EResourceState::Undefined;
}

CImageVk::CImageVk(CDeviceVk& p, CSwapChain::WeakRef swapChain)
    : Parent(p)
    , bIsSwapChainProxy(true)
    , SwapChain(swapChain)
{
    auto chain = std::static_pointer_cast<CSwapChainVk>(SwapChain.lock());
    CreateInfo.format = chain->GetChosenFormat();
    CreateInfo.arrayLayers = 1;
    CreateInfo.mipLevels = 1;
    DefaultState = EResourceState::Present;
    GlobalState = EResourceState::Undefined;
}

CImageVk::~CImageVk()
{
    if (bIsSwapChainProxy)
        return;

    if (!ImageAlloc)
        vkDestroyImage(Parent.GetVkDevice(), Image, nullptr);
    else
        vmaDestroyImage(Parent.GetAllocator(), Image, ImageAlloc);
}

VkImage CImageVk::GetVkImage() const
{
    if (bIsSwapChainProxy)
    {
        auto chain = std::static_pointer_cast<CSwapChainVk>(SwapChain.lock());
        if (chain->AcquiredImages.empty())
            throw CRHIRuntimeError("Did you forget to acquire image from the swapchain?");
        return chain->GetVkImages()[chain->AcquiredImages.front().first];
    }

    return Image;
}

bool CImageVk::IsConcurrentAccess() const
{
    if (bIsSwapChainProxy)
        return false;
    return CreateInfo.sharingMode;
}

void CImageVk::CopyFrom(const void* mem) { throw "unimplemented"; }

VkImageCreateInfo CImageVk::GetCreateInfo() const { return CreateInfo; }

EImageUsageFlags CImageVk::GetUsageFlags() const { return UsageFlags; }

EResourceState CImageVk::GetDefaultState() const { return DefaultState; }

EResourceState CImageVk::GetGlobalState() const { return GlobalState; }

void CImageVk::SetGlobalState(EResourceState state) { GlobalState = state; }

}
