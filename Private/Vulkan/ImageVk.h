#pragma once
#include "RHICommon.h"
#include "Resources.h"
#include "VkCommon.h"

namespace RHI
{

class CImageVk : public CImage
{
public:
    CImageVk(CDeviceVk& p, VkImage image, VmaAllocation alloc, const VkImageCreateInfo& createInfo,
             EImageUsageFlags usage, EResourceState defaultState);
    ~CImageVk();

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

    VkImage Image;
    VmaAllocation ImageAlloc;

private:
    CDeviceVk& Parent;
    VkImageCreateInfo CreateInfo;
    EImageUsageFlags UsageFlags;
    EResourceState DefaultState;
    EResourceState GlobalState;
};

} /* namespace RHI */
