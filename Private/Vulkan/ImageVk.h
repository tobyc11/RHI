#pragma once
#include "AccessTracker.h"
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

    virtual VkImage GetVkImage() const = 0;
    virtual VkFormat GetVkFormat() const = 0; // As opposed to RHI EFormat
    virtual bool IsConcurrentAccess() const = 0;
    virtual bool IsSwapChainProxy() const = 0;

    // Access tracking for barrier deduction
    void InitializeAccess(VkAccessFlags access, VkPipelineStageFlags stages, VkImageLayout layout);
    /// Transition a subset of this image to new access record. Inserts the barriers into cmdBuffer
    void TransitionAccess(VkCommandBuffer cmdBuffer, const CImageSubresourceRange& range,
                          const CAccessRecord& accessRecord);
    /// Doesn't do any transition, but updates the LastAccess map
    void UpdateAccess(const CImageSubresourceRange& range, const CAccessRecord& accessRecord);

    bool IsTrackingDisabled() const { return bIsTrackingDisabled; }
    void SetTrackingDisabled(bool value) { bIsTrackingDisabled = value; }

protected:
    CImageVk() = default;

private:
    std::map<CImageSubresourceRange, CAccessRecord> LastAccess;
    bool bIsTrackingDisabled = false;
};

class CSwapChainImageVk : public CImageVk
{
public:
    CSwapChainImageVk(CDeviceVk& p, CSwapChain::WeakRef swapChain);

    // CImage interface
    EFormat GetFormat() const;
    EImageUsageFlags GetUsageFlags() const;
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    uint32_t GetDepth() const { return 1; }
    uint32_t GetMipLevels() const { return 1; }
    uint32_t GetArrayLayers() const { return 1; }
    uint32_t GetSampleCount() const { return 1; }

    // CImageVk interface
    VkImage GetVkImage() const;
    VkFormat GetVkFormat() const;
    bool IsConcurrentAccess() const { return false; }
    bool IsSwapChainProxy() const { return true; }

    CSwapChain::WeakRef GetSwapChain() const { return SwapChain; }

private:
    CSwapChain::WeakRef SwapChain;
};

class CMemoryImageVk : public CImageVk
{
public:
    CMemoryImageVk(CDeviceVk& p, VkImage image, VmaAllocation alloc,
                   const VkImageCreateInfo& createInfo, EImageUsageFlags usage,
                   EResourceState defaultState);
    ~CMemoryImageVk();

    // CImage interface
    EFormat GetFormat() const { return static_cast<EFormat>(GetCreateInfo().format); }
    EImageUsageFlags GetUsageFlags() const;
    uint32_t GetWidth() const { return GetCreateInfo().extent.width; }
    uint32_t GetHeight() const { return GetCreateInfo().extent.height; }
    uint32_t GetDepth() const { return GetCreateInfo().extent.depth; }
    uint32_t GetMipLevels() const { return GetCreateInfo().mipLevels; }
    uint32_t GetArrayLayers() const { return GetCreateInfo().arrayLayers; }
    uint32_t GetSampleCount() const { return GetCreateInfo().samples; }

    // CImageVk interface
    VkImage GetVkImage() const;
    VkFormat GetVkFormat() const { return GetCreateInfo().format; }
    bool IsConcurrentAccess() const;
    bool IsSwapChainProxy() const { return false; }

    VkImageCreateInfo GetCreateInfo() const;
    EResourceState GetDefaultState() const;

private:
    CDeviceVk& Parent;

    VkImage Image = VK_NULL_HANDLE;
    VmaAllocation ImageAlloc = VK_NULL_HANDLE;

    VkImageCreateInfo CreateInfo;
    EImageUsageFlags UsageFlags {};
    EResourceState DefaultState;
};

} /* namespace RHI */
