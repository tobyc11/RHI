#pragma once
#include "Resources.h"
#include "VkCommon.h"
#include "VkHelpers.h"
#include <map>

namespace RHI
{

class CImageVk;
class CBufferVk;

struct CBufferRange
{
    CBufferVk* Buffer;
    size_t Offset;
    size_t Size;

    bool operator<(const CBufferRange& rhs) const
    {
        if (Buffer < rhs.Buffer)
            return true;
        if (Buffer > rhs.Buffer)
            return false;
        if (Offset < rhs.Offset)
            return true;
        if (Offset > rhs.Offset)
            return false;
        if (Size < rhs.Size)
            return true;
        if (Size > rhs.Size)
            return false;
        return false;
    }

    bool operator==(const CBufferRange& rhs) const
    {
        return Buffer == rhs.Buffer && Offset == rhs.Offset && Size == rhs.Size;
    }
};

struct CImageRange
{
    CImageVk* Image;
    CImageSubresourceRange Range;

    bool operator<(const CImageRange& rhs) const
    {
        if (Image < rhs.Image)
            return true;
        if (Image > rhs.Image)
            return false;
        if (Range < rhs.Range)
            return true;
        return false;
    }

    bool operator==(const CImageRange& rhs) const
    {
        return Image == rhs.Image && Range == rhs.Range;
    }
};

struct CAccessRecord
{
    VkAccessFlags AccessType;
    VkPipelineStageFlags Stages;
    VkImageLayout ImageLayout;

    bool IsRead() const;
    bool IsWrite() const;
};

// Tracks resource access for a certain time period (usually a command buffer)
class CAccessTracker
{
public:
    static bool CalcOverlap(const CImageSubresourceRange& range1,
                            const CImageSubresourceRange& range2, uint32_t& top, uint32_t& bottom,
                            uint32_t& left, uint32_t& right);
    static void InsertImageBarrier(VkCommandBuffer cmdBuffer, CImageVk* image,
                                   const CImageSubresourceRange& range,
                                   const CAccessRecord& oldAccess, const CAccessRecord& newAccess);

    void TransitionBuffer(CBufferVk* buffer, size_t offset, size_t size, VkAccessFlags access,
                          VkPipelineStageFlags stages);
    void TransitionImageState(VkCommandBuffer cmdBuffer, CImageVk* image,
                              const CImageSubresourceRange& range, EResourceState targetState,
                              bool isTransferQueue = false);
    void TransitionImage(VkCommandBuffer cmdBuffer, CImageVk* image,
                         const CImageSubresourceRange& range, VkAccessFlags access,
                         VkPipelineStageFlags stages, VkImageLayout layout);

    void DeployAllBarriers(VkCommandBuffer cmdBuffer);

    // Merge two access trackers together, and record the intermediate transitions
    void Merge(VkCommandBuffer cmdBuffer, const CAccessTracker& rhs);

    void Clear()
    {
        ImageFirstAccess.clear();
        ImageLastAccess.clear();
    }

private:
    void HandleImageFirstAccess(CImageVk* image, const CImageSubresourceRange& range,
                                const CAccessRecord& record);
    void HandleImageLastAccess(VkCommandBuffer cmdBuffer, CImageVk* image,
                               const CImageSubresourceRange& range, const CAccessRecord& record);

    // Not tracking buffers for now
    // std::map<CBufferRange, CAccessRecord> BufferFirstAccess;
    // std::map<CBufferRange, CAccessRecord> BufferLastAccess;

    std::map<CImageRange, CAccessRecord> ImageFirstAccess;
    std::map<CImageRange, CAccessRecord> ImageLastAccess;
};

}
