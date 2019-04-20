#include "ImageVk.h"
#include "CommandContextVk.h"
#include "DeviceVk.h"
#include "SwapChainVk.h"

namespace RHI
{

void CImageVk::InitializeAccess(VkAccessFlags access, VkPipelineStageFlags stages,
                                VkImageLayout layout)
{
    LastAccess.clear();
    CImageSubresourceRange entireImage;
    entireImage.BaseArrayLayer = 0;
    entireImage.BaseMipLevel = 0;
    entireImage.LayerCount = GetArrayLayers();
    entireImage.LevelCount = GetMipLevels();
    CAccessRecord record { access, stages, layout };
    LastAccess.emplace(entireImage, record);
}

void CImageVk::TransitionAccess(VkCommandBuffer cmdBuffer, const CImageSubresourceRange& range,
                                const CAccessRecord& accessRecord)
{
    if (LastAccess.empty())
        throw "CImageVk Access tracking is not initialized";

    for (const auto& pair : LastAccess)
    {
        uint32_t top, bottom, left, right;
        if (CAccessTracker::CalcOverlap(pair.first, range, top, bottom, left, right))
        {
            CImageSubresourceRange overlapRange;
            overlapRange.BaseMipLevel = top;
            overlapRange.LevelCount = bottom - top + 1;
            overlapRange.BaseArrayLayer = left;
            overlapRange.LayerCount = right - left + 1;
            CAccessTracker::InsertImageBarrier(cmdBuffer, this, overlapRange, pair.second,
                                               accessRecord);
        }
    }
}

void CImageVk::UpdateAccess(const CImageSubresourceRange& range, const CAccessRecord& accessRecord)
{
    if (LastAccess.empty())
        throw "CImageVk Access tracking is not initialized";

    // This function is largely identical to CAccessTracker::HandleImageLastAccess
    // Except we don't insert any actual barrier here
    auto iter = LastAccess.begin();

    while (iter != LastAccess.end())
    {
        uint32_t top, bottom, left, right;
        if (!CAccessTracker::CalcOverlap(iter->first, range, top, bottom, left, right))
        {
            iter++;
            continue;
        }

        // Split the old region into 4 and remove the overlapping one from the store
        // NOTE: my* is actually iter
        uint32_t iterTop = iter->first.BaseMipLevel;
        uint32_t iterLeft = iter->first.BaseArrayLayer;
        uint32_t iterBottom = iterTop + iter->first.LevelCount - 1;
        uint32_t iterRight = iterLeft + iter->first.LayerCount - 1;

        // Erase the region right now, we are gonna add the non-overlapping quadrants back in
        CAccessRecord iterAccess = iter->second;
        LastAccess.erase(iter++);

        // No need to continue if completely overlap
        if (top == iterTop && bottom == iterBottom && left == iterLeft && right == iterRight)
            continue;

        /*
         2 | 1
        -------
         3 | 4
        */
        uint32_t overlappingQuadrant;
        uint32_t midVertical, midHorizontal;
        if (top == iterTop)
        {
            midVertical = bottom;
            if (left == iterLeft)
            {
                midHorizontal = right;
                overlappingQuadrant = 2;
            }
            else
            {
                midHorizontal = left - 1;
                overlappingQuadrant = 1;
            }
        }
        else
        {
            midVertical = top - 1;
            if (left == iterLeft)
            {
                midHorizontal = right;
                overlappingQuadrant = 3;
            }
            else
            {
                midHorizontal = left - 1;
                overlappingQuadrant = 4;
            }
        }

        // Add the non-overlapping regions back in
        if (overlappingQuadrant != 2)
        {
            CImageSubresourceRange quadRange;
            quadRange.BaseMipLevel = iterTop;
            quadRange.LevelCount = midVertical - iterTop + 1;
            quadRange.BaseArrayLayer = iterLeft;
            quadRange.LayerCount = midHorizontal - iterLeft + 1;
            if (quadRange.LevelCount > 0 && quadRange.LayerCount > 0)
                LastAccess.emplace(quadRange, iterAccess);
        }
        if (overlappingQuadrant != 1)
        {
            CImageSubresourceRange quadRange;
            quadRange.BaseMipLevel = iterTop;
            quadRange.LevelCount = midVertical - iterTop + 1;
            quadRange.BaseArrayLayer = midHorizontal + 1;
            quadRange.LayerCount = iterRight - midHorizontal;
            if (quadRange.LevelCount > 0 && quadRange.LayerCount > 0)
                LastAccess.emplace(quadRange, iterAccess);
        }
        if (overlappingQuadrant != 3)
        {
            CImageSubresourceRange quadRange;
            quadRange.BaseMipLevel = midVertical + 1;
            quadRange.LevelCount = iterBottom - midVertical;
            quadRange.BaseArrayLayer = iterLeft;
            quadRange.LayerCount = midHorizontal - iterLeft + 1;
            if (quadRange.LevelCount > 0 && quadRange.LayerCount > 0)
                LastAccess.emplace(quadRange, iterAccess);
        }
        if (overlappingQuadrant != 4)
        {
            CImageSubresourceRange quadRange;
            quadRange.BaseMipLevel = midVertical + 1;
            quadRange.LevelCount = iterBottom - midVertical;
            quadRange.BaseArrayLayer = midHorizontal + 1;
            quadRange.LayerCount = iterRight - midHorizontal;
            if (quadRange.LevelCount > 0 && quadRange.LayerCount > 0)
                LastAccess.emplace(quadRange, iterAccess);
        }
    }
    // We insert the whole range if we've found no overlapping
    LastAccess.emplace(range, accessRecord);
}

CSwapChainImageVk::CSwapChainImageVk(CDeviceVk& p, CSwapChain::WeakRef swapChain)
    : SwapChain(swapChain)
{
    InitializeAccess(0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_IMAGE_LAYOUT_UNDEFINED);
}

EFormat CSwapChainImageVk::GetFormat() const
{
    auto scImpl = std::static_pointer_cast<CSwapChainVk>(SwapChain.lock());
    return static_cast<EFormat>(scImpl->GetChosenFormat());
}

EImageUsageFlags CSwapChainImageVk::GetUsageFlags() const { return EImageUsageFlags::RenderTarget; }

uint32_t CSwapChainImageVk::GetWidth() const
{
    uint32_t width, height;
    auto scImpl = std::static_pointer_cast<CSwapChainVk>(SwapChain.lock());
    scImpl->GetSize(width, height);
    return width;
}

uint32_t CSwapChainImageVk::GetHeight() const
{
    uint32_t width, height;
    auto scImpl = std::static_pointer_cast<CSwapChainVk>(SwapChain.lock());
    scImpl->GetSize(width, height);
    return height;
}

VkImage CSwapChainImageVk::GetVkImage() const
{
    auto chain = std::static_pointer_cast<CSwapChainVk>(SwapChain.lock());
    if (chain->AcquiredImages.empty())
        throw CRHIRuntimeError("CSwapChainImageVk::GetVkImage failed: did you forget to acquire "
                               "image from the swapchain?");
    return chain->GetVkImages()[chain->AcquiredImages.front().first];
}

VkFormat CSwapChainImageVk::GetVkFormat() const
{
    auto scImpl = std::static_pointer_cast<CSwapChainVk>(SwapChain.lock());
    return scImpl->GetChosenFormat();
}

CMemoryImageVk::CMemoryImageVk(CDeviceVk& p, VkImage image, VmaAllocation alloc,
                               const VkImageCreateInfo& createInfo, EImageUsageFlags usage,
                               EResourceState defaultState)
    : Parent(p)
    , Image(image)
    , ImageAlloc(alloc)
    , CreateInfo(createInfo)
    , UsageFlags(usage)
    , DefaultState(defaultState)
{
    InitializeAccess(0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, CreateInfo.initialLayout);
}

CMemoryImageVk::~CMemoryImageVk()
{
    if (!ImageAlloc)
        vkDestroyImage(Parent.GetVkDevice(), Image, nullptr);
    else
        vmaDestroyImage(Parent.GetAllocator(), Image, ImageAlloc);
}

EImageUsageFlags CMemoryImageVk::GetUsageFlags() const { return UsageFlags; }

VkImage CMemoryImageVk::GetVkImage() const { return Image; }

bool CMemoryImageVk::IsConcurrentAccess() const { return CreateInfo.sharingMode; }

VkImageCreateInfo CMemoryImageVk::GetCreateInfo() const { return CreateInfo; }

EResourceState CMemoryImageVk::GetDefaultState() const { return DefaultState; }

}
