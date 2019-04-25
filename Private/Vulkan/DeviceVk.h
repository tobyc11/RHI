#pragma once
#include "Device.h"

#include "BufferVk.h"
#include "CommandContextVk.h"
#include "DescriptorSetLayoutCacheVk.h"
#include "VkCommon.h"

#include <mutex>
#include <queue>

namespace RHI
{

class CDeviceVk : public CDevice
{
public:
    typedef std::shared_ptr<CDeviceVk> Ref;

    CDeviceVk(EDeviceCreateHints hints);
    ~CDeviceVk() override;

    CImage::Ref InternalCreateImage(VkImageType type, EFormat format, EImageUsageFlags usage,
                                    uint32_t width, uint32_t height, uint32_t depth,
                                    uint32_t mipLevels, uint32_t arrayLayers, uint32_t sampleCount,
                                    const void* initialData);

    // Resources and resource views
    CBuffer::Ref CreateBuffer(size_t size, EBufferUsageFlags usage,
                              const void* initialData = nullptr);
    CImage::Ref CreateImage1D(EFormat format, EImageUsageFlags usage, uint32_t width,
                              uint32_t mipLevels = 1, uint32_t arrayLayers = 1,
                              uint32_t sampleCount = 1, const void* initialData = nullptr);
    CImage::Ref CreateImage2D(EFormat format, EImageUsageFlags usage, uint32_t width,
                              uint32_t height, uint32_t mipLevels = 1, uint32_t arrayLayers = 1,
                              uint32_t sampleCount = 1, const void* initialData = nullptr);
    CImage::Ref CreateImage3D(EFormat format, EImageUsageFlags usage, uint32_t width,
                              uint32_t height, uint32_t depth, uint32_t mipLevels = 1,
                              uint32_t arrayLayers = 1, uint32_t sampleCount = 1,
                              const void* initialData = nullptr);
    CImageView::Ref CreateImageView(const CImageViewDesc& desc, CImage::Ref image);

    // Shader and resource binding
    CShaderModule::Ref CreateShaderModule(size_t size, const void* pCode);
    // CDescriptorSetLayout::Ref CreateDescriptorSetLayout(const CDescriptorSetLayoutDesc& desc);
    // CPipelineLayout::Ref CreatePipelineLayout(const CPipelineLayoutDesc& desc);

    // CDescriptorPool::Ref CreateDescriptorPool();
    // CDescriptorSet::Ref CreateDescriptorSet(CDescriptorPool& pool, CDescriptorSetLayout& layout);

    // States
    CRenderPass::Ref CreateRenderPass(const CRenderPassDesc& desc);
    CPipeline::Ref CreatePipeline(const CPipelineDesc& desc);
    CSampler::Ref CreateSampler(const CSamplerDesc& desc);

    // Command submission
    IImmediateContext::Ref GetImmediateContext();
    CCommandList::Ref CreateCommandList();
    ICopyContext::Ref CreateCopyContext(CCommandList& cmdList);
    IComputeContext::Ref CreateComputeContext(CCommandList& cmdList);
    IRenderPassContext::Ref CreateRenderPassContext(CCommandList& cmdList, CRenderPass& renderPass,
                                                    const std::vector<CClearValue>& clearValues);

    CSwapChain::Ref CreateSwapChain(const CPresentationSurfaceDesc& info, EFormat format);
    void WaitIdle();

    // Vulkan specific getters
    VkInstance GetVkInstance() const;
    VkDevice GetVkDevice() const { return Device; }
    VkPhysicalDevice GetVkPhysicalDevice() const { return PhysicalDevice; }
    const VkPhysicalDeviceLimits& GetVkLimits() const { return Properties.limits; }

    // Otherwise transfer and graphics are the same queue
    bool IsTransferQueueSeparate() const
    {
        return QueueFamilies[QT_TRANSFER] != QueueFamilies[QT_GRAPHICS];
    }
    bool IsComputeQueueSeparate() const
    {
        return QueueFamilies[QT_COMPUTE] != QueueFamilies[QT_GRAPHICS];
    }

    // Getters for global objects
    uint32_t GetQueueFamily(uint32_t type) const { return QueueFamilies[type]; }
    VkQueue GetVkQueue(uint32_t type) const { return Queues[type][0]; }
    VmaAllocator GetAllocator() const { return Allocator; }

    CDescriptorSetLayoutCacheVk* GetDescriptorSetLayoutCache() const
    {
        return DescriptorSetLayoutCache.get();
    }
    CPersistentMappedRingBuffer* GetHugeConstantBuffer() const { return HugeConstantBuffer.get(); }
    CSubmissionTracker& GetSubmissionTracker() { return SubmissionTracker; }
    CCommandContextVk::Ref MakeTransientContext(EQueueType qt);
    VkPipelineCache GetPipelineCache() const { return PipelineCache; }

private:
    VkDevice Device;

    // NOTE: according to some AMD doc https://gpuopen.com/concurrent-execution-asynchronous-queues/
    //   it's best to stick to one queue per family for current GPUs
    VkPhysicalDevice PhysicalDevice;
    VkPhysicalDeviceProperties Properties;

    // Global objects
    uint32_t QueueFamilies[NUM_QUEUE_TYPES];
    std::vector<VkQueue> Queues[NUM_QUEUE_TYPES];
    VmaAllocator Allocator;
    std::unique_ptr<CDescriptorSetLayoutCacheVk> DescriptorSetLayoutCache;
    std::unique_ptr<CPersistentMappedRingBuffer> HugeConstantBuffer;
    CSubmissionTracker SubmissionTracker;
    CCommandContextVk::Ref ImmediateContext;
    VkPipelineCache PipelineCache;

    // Every job submission collects those, and deletes them when job is finished
    friend class CSubmissionTracker;
    std::vector<std::pair<VkBuffer, VmaAllocation>> PendingDeletionBuffers;
    std::vector<std::pair<VkImage, VmaAllocation>> PendingDeletionImages;
};

} /* namespace RHI */
