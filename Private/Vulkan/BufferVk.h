#pragma once
#include "Resources.h"
#include "VkCommon.h"
#include <queue>

namespace RHI
{

class CBufferVk : public CBuffer
{
public:
    typedef std::shared_ptr<CBufferVk> Ref;

    CBufferVk(CDeviceVk& p, size_t size, EBufferUsageFlags usage, const void* initialData);
    ~CBufferVk() override;

    void* Map(size_t offset, size_t size);
    void Unmap();

    VkBuffer Buffer;
    VmaAllocation Allocation;

private:
    CDeviceVk& Parent;
};

class CPersistentMappedRingBuffer
{
public:
    CPersistentMappedRingBuffer(CDeviceVk& p, size_t size, VkBufferUsageFlags usage);
    ~CPersistentMappedRingBuffer();
    CPersistentMappedRingBuffer(const CPersistentMappedRingBuffer&) = delete;
    CPersistentMappedRingBuffer(CPersistentMappedRingBuffer&&) = delete;
    CPersistentMappedRingBuffer& operator=(const CPersistentMappedRingBuffer&) = delete;
    CPersistentMappedRingBuffer& operator=(CPersistentMappedRingBuffer&&) = delete;

    void* Allocate(size_t size, size_t alignment, size_t& outOffset);
    void MarkBlockEnd();
    void FreeBlock();

    VkBuffer GetHandle() const { return Handle; }

private:
    CDeviceVk& Parent;

    VkBuffer Handle;
    VmaAllocation Allocation;

    size_t TotalSize;
    size_t Remaining;

    struct BlockInfo
    {
        size_t Begin = 0;
        size_t End = 0;
    };
    BlockInfo CurrBlock;
    std::queue<BlockInfo> AllocatedBlocks;

    void* MappedData;
};

} /* namespace RHI */
