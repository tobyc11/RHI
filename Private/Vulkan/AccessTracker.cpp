#include "AccessTracker.h"
#include "BufferVk.h"
#include "ImageVk.h"
#include <algorithm>

namespace RHI
{

bool CAccessRecord::IsRead() const
{
    auto allReadBits = VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT
        | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT
        | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT
        | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
        | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_HOST_READ_BIT | VK_ACCESS_MEMORY_READ_BIT;
    return AccessType & allReadBits;
}

bool CAccessRecord::IsWrite() const
{
    auto allWriteBits = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT
        | VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
    return AccessType & allWriteBits;
}

bool CAccessTracker::CalcOverlap(const CImageSubresourceRange& range1,
                                 const CImageSubresourceRange& range2, uint32_t& top,
                                 uint32_t& bottom, uint32_t& left, uint32_t& right)
{
    top = std::max(range1.BaseMipLevel, range2.BaseMipLevel);
    bottom = std::min(range1.BaseMipLevel + range1.LevelCount - 1,
                      range2.BaseMipLevel + range2.LevelCount - 1);
    left = std::max(range1.BaseArrayLayer, range2.BaseArrayLayer);
    right = std::min(range1.BaseArrayLayer + range1.LayerCount - 1,
                     range2.BaseArrayLayer + range2.LayerCount - 1);

    if (bottom < top || right < left)
        return false;
    return true;
}

void CAccessTracker::InsertImageBarrier(VkCommandBuffer cmdBuffer, CImageVk* image,
                                        const CImageSubresourceRange& range,
                                        const CAccessRecord& oldAccess,
                                        const CAccessRecord& newAccess)
{
    // Nop if read-read
    if (!oldAccess.IsWrite() && !newAccess.IsWrite()
        && oldAccess.ImageLayout == newAccess.ImageLayout)
        return;

    // WAR only needs an execution barrier
    if (oldAccess.IsRead() && oldAccess.ImageLayout == newAccess.ImageLayout)
    {
        vkCmdPipelineBarrier(cmdBuffer, oldAccess.Stages, newAccess.Stages, 0, 0, nullptr, 0,
                             nullptr, 0, nullptr);
        return;
    }

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcAccessMask = oldAccess.AccessType;
    barrier.dstAccessMask = newAccess.AccessType;
    barrier.oldLayout = oldAccess.ImageLayout;
    barrier.newLayout = newAccess.ImageLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image->GetVkImage();
    barrier.subresourceRange.aspectMask = GetImageAspectFlags(image->GetVkFormat());
    barrier.subresourceRange.baseArrayLayer = range.BaseArrayLayer;
    barrier.subresourceRange.baseMipLevel = range.BaseMipLevel;
    barrier.subresourceRange.layerCount = range.LayerCount;
    barrier.subresourceRange.levelCount = range.LevelCount;
    vkCmdPipelineBarrier(cmdBuffer, oldAccess.Stages, newAccess.Stages, 0, 0, nullptr, 0, nullptr,
                         1, &barrier);
}

void CAccessTracker::TransitionBuffer(CBufferVk* buffer, size_t offset, size_t size,
                                      VkAccessFlags access, VkPipelineStageFlags stages)
{
    throw "unimplemented";
}

void CAccessTracker::TransitionImageState(VkCommandBuffer cmdBuffer, CImageVk* image,
                                          const CImageSubresourceRange& range,
                                          EResourceState targetState, bool isTransferQueue)
{
    auto dstAccess = StateToAccessMask(targetState);
    auto dstStages = StateToShaderStageMask(targetState, false);
    if (isTransferQueue && targetState != EResourceState::CopyDest
        && targetState != EResourceState::CopySource)
    {
        // Only do layout transitions if we are on the transfer queue and the target state is not
        // supported on the transfer queue
        dstAccess = 0;
        dstStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
    auto dstLayout = StateToImageLayout(targetState);
    TransitionImage(cmdBuffer, image, range, dstAccess, dstStages, dstLayout);
}

void CAccessTracker::TransitionImage(VkCommandBuffer cmdBuffer, CImageVk* image,
                                     const CImageSubresourceRange& range, VkAccessFlags access,
                                     VkPipelineStageFlags stages, VkImageLayout layout)
{
    CAccessRecord currAccess;
    currAccess.AccessType = access;
    currAccess.Stages = stages;
    currAccess.ImageLayout = layout;
    HandleImageFirstAccess(image, range, currAccess);
    HandleImageLastAccess(cmdBuffer, image, range, currAccess);
}

void CAccessTracker::DeployAllBarriers(VkCommandBuffer cmdBuffer)
{
    // Transition all relevant images to the needed state
    for (const auto& iter : ImageFirstAccess)
    {
        if (iter.second.ImageLayout == VK_IMAGE_LAYOUT_UNDEFINED
            || iter.second.ImageLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
            continue;
        iter.first.Image->TransitionAccess(cmdBuffer, iter.first.Range, iter.second);
    }
    for (const auto& iter : ImageLastAccess)
    {
        iter.first.Image->UpdateAccess(iter.first.Range, iter.second);
    }
}

void CAccessTracker::HandleImageFirstAccess(CImageVk* image, const CImageSubresourceRange& range,
                                            const CAccessRecord& record)
{
    auto iter = ImageFirstAccess.lower_bound(CImageRange { image, CImageSubresourceRange() });
    if (iter == ImageFirstAccess.end() || iter->first.Image != image)
    {
        // This image is never accessed before
        ImageFirstAccess.emplace(CImageRange { image, range }, record);
        return;
    }

    // Rules for access record updating: for each subresource, first access should not change
    //   but last access should always change
    while (iter != ImageFirstAccess.end() && iter->first.Image == image)
    {
        if (!range.Overlaps(iter->first.Range))
        {
            iter++;
            continue;
        }

        // Calculate the overlapping region, vertical axis is miplevel
        uint32_t top, bottom, left, right;
        CalcOverlap(range, iter->first.Range, top, bottom, left, right);

        // Split the new region into 4 and recursively add them into the record store
        uint32_t myTop = range.BaseMipLevel;
        uint32_t myLeft = range.BaseArrayLayer;
        uint32_t myBottom = myTop + range.LevelCount - 1;
        uint32_t myRight = myLeft + range.LayerCount - 1;

        // If completely overlap, we don't need to do anything
        if (top == myTop && bottom == myBottom && left == myLeft && right == myRight)
            return;

        /*
         2 | 1
        -------
         3 | 4
        */
        uint32_t overlappingQuadrant;
        uint32_t midVertical, midHorizontal;
        if (top == myTop)
        {
            midVertical = bottom;
            if (left == myLeft)
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
            if (left == myLeft)
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

        // Recursively add the non-overlapping regions
        if (overlappingQuadrant != 2)
        {
            CImageSubresourceRange quadRange;
            quadRange.BaseMipLevel = myTop;
            quadRange.LevelCount = midVertical - myTop + 1;
            quadRange.BaseArrayLayer = myLeft;
            quadRange.LayerCount = midHorizontal - myLeft + 1;
            // This if statement handles degenerate quads
            if (quadRange.LevelCount > 0 && quadRange.LayerCount > 0)
                HandleImageFirstAccess(image, quadRange, record);
        }
        if (overlappingQuadrant != 1)
        {
            CImageSubresourceRange quadRange;
            quadRange.BaseMipLevel = myTop;
            quadRange.LevelCount = midVertical - myTop + 1;
            quadRange.BaseArrayLayer = midHorizontal + 1;
            quadRange.LayerCount = myRight - midHorizontal;
            if (quadRange.LevelCount > 0 && quadRange.LayerCount > 0)
                HandleImageFirstAccess(image, quadRange, record);
        }
        if (overlappingQuadrant != 3)
        {
            CImageSubresourceRange quadRange;
            quadRange.BaseMipLevel = midVertical + 1;
            quadRange.LevelCount = myBottom - midVertical;
            quadRange.BaseArrayLayer = myLeft;
            quadRange.LayerCount = midHorizontal - myLeft + 1;
            if (quadRange.LevelCount > 0 && quadRange.LayerCount > 0)
                HandleImageFirstAccess(image, quadRange, record);
        }
        if (overlappingQuadrant != 4)
        {
            CImageSubresourceRange quadRange;
            quadRange.BaseMipLevel = midVertical + 1;
            quadRange.LevelCount = myBottom - midVertical;
            quadRange.BaseArrayLayer = midHorizontal + 1;
            quadRange.LayerCount = myRight - midHorizontal;
            if (quadRange.LevelCount > 0 && quadRange.LayerCount > 0)
                HandleImageFirstAccess(image, quadRange, record);
        }
        return;
    }
    // We insert the whole range if we've found no overlapping
    ImageFirstAccess.emplace(CImageRange { image, range }, record);
}

void CAccessTracker::HandleImageLastAccess(VkCommandBuffer cmdBuffer, CImageVk* image,
                                           const CImageSubresourceRange& range,
                                           const CAccessRecord& record)
{
    auto iter = ImageLastAccess.lower_bound(CImageRange { image, CImageSubresourceRange() });
    if (iter == ImageLastAccess.end() || iter->first.Image != image)
    {
        // This image is never accessed before
        ImageLastAccess.emplace(CImageRange { image, range }, record);
        return;
    }

    while (iter != ImageLastAccess.end() && iter->first.Image == image)
    {
        if (!range.Overlaps(iter->first.Range))
        {
            iter++;
            continue;
        }

        // Calculate the overlapping region, vertical axis is miplevel
        uint32_t top, bottom, left, right;
        CalcOverlap(range, iter->first.Range, top, bottom, left, right);

        // Go ahead and transition the overlapping region
        if (cmdBuffer)
        {
            CImageSubresourceRange overlapRange;
            overlapRange.BaseMipLevel = top;
            overlapRange.LevelCount = bottom - top + 1;
            overlapRange.BaseArrayLayer = left;
            overlapRange.LayerCount = right - left + 1;
            InsertImageBarrier(cmdBuffer, image, overlapRange, iter->second, record);
        }

        // Split the old region into 4 and remove the overlapping one from the store
        // NOTE: my* is actually iter
        uint32_t myTop = iter->first.Range.BaseMipLevel;
        uint32_t myLeft = iter->first.Range.BaseArrayLayer;
        uint32_t myBottom = myTop + iter->first.Range.LevelCount - 1;
        uint32_t myRight = myLeft + iter->first.Range.LayerCount - 1;

        // Erase the region right now, we are gonna add the non-overlapping quadrants back in
        CAccessRecord myAccess = iter->second;
        ImageLastAccess.erase(iter++);

        // No need to continue if the target region is completely contained
        if (top == myTop && bottom == myBottom && left == myLeft && right == myRight)
            continue;

        /*
         2 | 1
        -------
         3 | 4
        */
        uint32_t overlappingQuadrant;
        uint32_t midVertical, midHorizontal;
        if (top == myTop)
        {
            midVertical = bottom;
            if (left == myLeft)
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
            if (left == myLeft)
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
            quadRange.BaseMipLevel = myTop;
            quadRange.LevelCount = midVertical - myTop + 1;
            quadRange.BaseArrayLayer = myLeft;
            quadRange.LayerCount = midHorizontal - myLeft + 1;
            if (quadRange.LevelCount > 0 && quadRange.LayerCount > 0)
                ImageLastAccess.emplace(CImageRange { image, quadRange }, myAccess);
        }
        if (overlappingQuadrant != 1)
        {
            CImageSubresourceRange quadRange;
            quadRange.BaseMipLevel = myTop;
            quadRange.LevelCount = midVertical - myTop + 1;
            quadRange.BaseArrayLayer = midHorizontal + 1;
            quadRange.LayerCount = myRight - midHorizontal;
            if (quadRange.LevelCount > 0 && quadRange.LayerCount > 0)
                ImageLastAccess.emplace(CImageRange { image, quadRange }, myAccess);
        }
        if (overlappingQuadrant != 3)
        {
            CImageSubresourceRange quadRange;
            quadRange.BaseMipLevel = midVertical + 1;
            quadRange.LevelCount = myBottom - midVertical;
            quadRange.BaseArrayLayer = myLeft;
            quadRange.LayerCount = midHorizontal - myLeft + 1;
            if (quadRange.LevelCount > 0 && quadRange.LayerCount > 0)
                ImageLastAccess.emplace(CImageRange { image, quadRange }, myAccess);
        }
        if (overlappingQuadrant != 4)
        {
            CImageSubresourceRange quadRange;
            quadRange.BaseMipLevel = midVertical + 1;
            quadRange.LevelCount = myBottom - midVertical;
            quadRange.BaseArrayLayer = midHorizontal + 1;
            quadRange.LayerCount = myRight - midHorizontal;
            if (quadRange.LevelCount > 0 && quadRange.LayerCount > 0)
                ImageLastAccess.emplace(CImageRange { image, quadRange }, myAccess);
        }
    }
    // We insert the whole range if we've found no overlapping
    ImageLastAccess.emplace(CImageRange { image, range }, record);
}
}
