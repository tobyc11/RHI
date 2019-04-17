#include "BufferVk.h"
#include "DeviceVk.h"

namespace RHI
{

CBufferVk::CBufferVk(CDeviceVk& p, size_t size, EBufferUsageFlags usage, const void* initialData)
    : CBuffer(size, usage)
    , Parent(p)
{
    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = size;

    VmaAllocationCreateInfo allocInfo = {};

    bool gpuOnly = true;

    if (Any(usage, EBufferUsageFlags::VertexBuffer))
        bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (Any(usage, EBufferUsageFlags::IndexBuffer))
        bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (Any(usage, EBufferUsageFlags::ConstantBuffer))
    {
        bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        gpuOnly = false;
    }
    if (Any(usage, EBufferUsageFlags::Streaming))
        gpuOnly = false;

    if (gpuOnly)
    {
        bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    }
    else
    {
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    }

    vmaCreateBuffer(Parent.GetAllocator(), &bufferInfo, &allocInfo, &Buffer, &Allocation, nullptr);

    if (initialData)
    {
        // Prepare a staging buffer
        VkBufferCreateInfo stgbufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        stgbufferInfo.size = size;
        stgbufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        VmaAllocationCreateInfo stgallocInfo = {};
        stgallocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

        VkBuffer stagingBuffer;
        VmaAllocation stagingAlloc;

        vmaCreateBuffer(Parent.GetAllocator(), &stgbufferInfo, &stgallocInfo, &stagingBuffer,
                        &stagingAlloc, nullptr);

        void* mappedData;
        vmaMapMemory(Parent.GetAllocator(), stagingAlloc, &mappedData);
        memcpy(mappedData, initialData, size);
        vmaUnmapMemory(Parent.GetAllocator(), stagingAlloc);

        // Synchronously copy the content
        auto ctx = Parent.GetImmediateTransferCtx();
        auto cmdBuffer = ctx->GetBuffer();
        VkBufferCopy copy;
        copy.srcOffset = 0;
        copy.dstOffset = 0;
        copy.size = size;
        vkCmdCopyBuffer(cmdBuffer, stagingBuffer, Buffer, 1, &copy);
        ctx->Flush(true);
        Parent.PutImmediateTransferCtx(ctx);

        vmaDestroyBuffer(Parent.GetAllocator(), stagingBuffer, stagingAlloc);
    }
}

CBufferVk::~CBufferVk() { vmaDestroyBuffer(Parent.GetAllocator(), Buffer, Allocation); }

void* CBufferVk::Map(size_t offset, size_t size)
{
    void* result;
    vmaMapMemory(Parent.GetAllocator(), Allocation, &result);
    return static_cast<uint8_t*>(result) + offset;
}

void CBufferVk::Unmap() { vmaUnmapMemory(Parent.GetAllocator(), Allocation); }

CPersistentMappedRingBuffer::CPersistentMappedRingBuffer(CDeviceVk& p, size_t size,
                                                         VkBufferUsageFlags usage)
    : Parent(p)
    , TotalSize(size)
{
    Remaining = TotalSize;

    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = TotalSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    vmaCreateBuffer(Parent.GetAllocator(), &bufferInfo, &allocInfo, &Handle, &Allocation, nullptr);

    vmaMapMemory(Parent.GetAllocator(), Allocation, &MappedData);
}

CPersistentMappedRingBuffer::~CPersistentMappedRingBuffer()
{
    vmaUnmapMemory(Parent.GetAllocator(), Allocation);
    vmaDestroyBuffer(Parent.GetAllocator(), Handle, Allocation);
}

void* CPersistentMappedRingBuffer::Allocate(size_t size, size_t alignment, size_t& outOffset)
{
    if (CurrBlock.End + size + alignment > TotalSize)
    {
        size_t wastedSpace = TotalSize - CurrBlock.End;
        if (wastedSpace > Remaining)
            return nullptr; // Not enough free space to wrap around
        // Wrap around
        CurrBlock.End = 0;
        Remaining -= wastedSpace;
    }
    size_t allocOffset = (CurrBlock.End + alignment - 1) / alignment * alignment;
    size_t wastedOnAlighment = allocOffset - CurrBlock.End;
    if (allocOffset + size > TotalSize)
        return nullptr;

    Remaining = Remaining - wastedOnAlighment - size;
    CurrBlock.End = allocOffset + size;

    outOffset = allocOffset;
    return reinterpret_cast<void*>(reinterpret_cast<size_t>(MappedData) + allocOffset);
}

void CPersistentMappedRingBuffer::MarkBlockEnd()
{
    AllocatedBlocks.push(CurrBlock);
    CurrBlock.Begin = CurrBlock.End;
    if (CurrBlock.Begin == TotalSize)
        CurrBlock.Begin = 0;
    CurrBlock.End = CurrBlock.Begin;
}

void CPersistentMappedRingBuffer::FreeBlock()
{
    const auto& firstBlock = AllocatedBlocks.front();
    size_t blockSize = firstBlock.End - firstBlock.Begin;
    if (firstBlock.End < firstBlock.Begin)
        blockSize = firstBlock.End - firstBlock.Begin + TotalSize;
    Remaining += blockSize;
    assert(Remaining <= TotalSize);
    AllocatedBlocks.pop();
}

}
