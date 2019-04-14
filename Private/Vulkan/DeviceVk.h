#pragma once
#include "Device.h"

#include "CommandContextVk.h"
#include "DescriptorSetLayoutCacheVk.h"
#include "VkCommon.h"

#include <mutex>
#include <queue>

namespace RHI
{

enum EQueueType
{
    TransferQueue,
    ComputeQueue,
    GraphicsQueue,
    NumQueueType
};

struct CGPUJobInfo
{
    // Fill out these
    bool bIsFrame = false; // Whether this job is for a present
    std::vector<VkCommandBuffer> CmdBuffersInFlight;
    std::vector<CCommandContextVk::Ref> CmdContexts;
    std::vector<VkSemaphore> WaitSemaphores;
    std::vector<VkPipelineStageFlags> WaitStages;
    VkSemaphore SignalSemaphore;
    EQueueType QueueType;
    std::vector<std::function<void()>> DeferredDeleters;

    // Don't worry about this
    VkFence Fence;
    std::vector<std::pair<VkBuffer, VmaAllocation>> PendingDeletionBuffers;
    std::vector<std::pair<VkImage, VmaAllocation>> PendingDeletionImages;

    void AddCommandBuffer(VkCommandBuffer b, CCommandContextVk::Ref ctx)
    {
        CmdBuffersInFlight.push_back(b);
        CmdContexts.push_back(ctx);
    }

    void Reset()
    {
        CmdBuffersInFlight.clear();
        CmdContexts.clear();
    }
};

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
    IRenderContext::Ref GetImmediateContext();
    IRenderContext::Ref CreateDeferredContext();

    CSwapChain::Ref CreateSwapChain(const CPresentationSurfaceDesc& info, EFormat format);

    // Vulkan specific getters
    VkInstance GetVkInstance() const;
    VkDevice GetVkDevice() const { return Device; }
    VkPhysicalDevice GetVkPhysicalDevice() const { return PhysicalDevice; }
    const VkPhysicalDeviceLimits& GetVkLimits() const { return Properties.limits; }
    VmaAllocator GetAllocator() const { return Allocator; }
    VkQueue GetVkQueue(uint32_t type) const { return Queues[type][0]; }
    uint32_t GetQueueFamily(uint32_t type) const { return QueueFamilies[type]; }
    void SubmitJob(CGPUJobInfo jobInfo);
    void FinishOneJob(bool wait = false);
    CCommandContextVk::Ref GetImmediateTransferCtx()
    {
        ImmediateTransferMutex.lock();
        return ImmediateTransferCtx;
    }
    void PutImmediateTransferCtx(CCommandContextVk::Ref ref) { ImmediateTransferMutex.unlock(); }
    CDescriptorSetLayoutCacheVk* GetDescriptorSetLayoutCache() const
    {
        return DescriptorSetLayoutCache.get();
    }

private:
    VkDevice Device;

    // NOTE: according to some AMD doc https://gpuopen.com/concurrent-execution-asynchronous-queues/
    //   it's best to stick to one queue per family for current GPUs
    VkPhysicalDevice PhysicalDevice;
    VkPhysicalDeviceProperties Properties;
    uint32_t QueueFamilies[NumQueueType];
    std::vector<VkQueue> Queues[NumQueueType];

    std::mutex ImmediateTransferMutex;
    CCommandContextVk::Ref ImmediateTransferCtx; // Per thread??
    CCommandContextVk::Ref ImmediateGraphicsCtx;

    VmaAllocator Allocator;
    std::unique_ptr<CDescriptorSetLayoutCacheVk> DescriptorSetLayoutCache;

    // A ring buffer contains the jobs currently in flight
    std::mutex JobSubmitMutex;
    std::queue<CGPUJobInfo> JobQueue;
    uint32_t FrameJobCount = 0;
    static const uint32_t MaxJobsInFlight = 8;
    static const uint32_t MaxFramesInFlight = 1;

    // Every job submission collects those, and deletes them when job is finished
    std::vector<std::pair<VkBuffer, VmaAllocation>> PendingDeletionBuffers;
    std::vector<std::pair<VkImage, VmaAllocation>> PendingDeletionImages;
};

} /* namespace RHI */
