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
        //Prepare a staging buffer
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

        //Synchronously copy the content
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

}
