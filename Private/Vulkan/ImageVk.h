#pragma once
#include "RHICommon.h"
#include "Resources.h"
#include "SwapChain.h"
#include "VkCommon.h"
#include <queue>

namespace RHI
{

class CImageVk : public CImage
{
public:
    typedef std::shared_ptr<CImageVk> Ref;

    CImageVk(CDeviceVk& p, VkImage image, VmaAllocation alloc, const VkImageCreateInfo& createInfo,
             EImageUsageFlags usage, EResourceState defaultState);
    CImageVk(CDeviceVk& p, CSwapChain::WeakRef swapChain);
    ~CImageVk();

	VkImage GetVkImage() const;
    bool IsConcurrentAccess() const;

    void CopyFrom(const void* mem);
    EFormat GetFormat() const { return static_cast<EFormat>(GetCreateInfo().format); }
    EImageUsageFlags GetUsageFlags() const;
    uint32_t GetWidth() const { return GetCreateInfo().extent.width; }
    uint32_t GetHeight() const { return GetCreateInfo().extent.height; }
    uint32_t GetDepth() const { return GetCreateInfo().extent.depth; }
    uint32_t GetMipLevels() const { return GetCreateInfo().mipLevels; }
    uint32_t GetArrayLayers() const { return GetCreateInfo().arrayLayers; }
    uint32_t GetSampleCount() const { return GetCreateInfo().samples; }

    VkImageCreateInfo GetCreateInfo() const;
    EResourceState GetDefaultState() const;

    EResourceState GetGlobalState() const;
    void SetGlobalState(EResourceState state);

    // The image object could be a mere proxy for a swapchain, and does not hold any real image
    bool bIsSwapChainProxy = false;
    CSwapChain::WeakRef SwapChain;

private:
    CDeviceVk& Parent;

    VkImage Image = VK_NULL_HANDLE;
    VmaAllocation ImageAlloc = VK_NULL_HANDLE;

    VkImageCreateInfo CreateInfo;
    EImageUsageFlags UsageFlags {};
    EResourceState DefaultState;
    EResourceState GlobalState;
};

} /* namespace RHI */
