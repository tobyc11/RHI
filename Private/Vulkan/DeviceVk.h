#pragma once
#include "Device.h"

#include "BufferVk.h"
#include "CommandContextVk.h"
#include "CommandQueueVk.h"
#include "DescriptorSet.h"
#include "VkCommon.h"

#include <mutex>
#include <queue>

namespace RHI
{

class CDeviceVk : public CDevice
{
public:
    typedef std::shared_ptr<CDeviceVk> Ref;

    explicit CDeviceVk(EDeviceCreateHints hints);
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
    CDescriptorSetLayout::Ref
    CreateDescriptorSetLayout(const std::vector<CDescriptorSetLayoutBinding>& bindings);
    CPipelineLayout::Ref
    CreatePipelineLayout(const std::vector<CDescriptorSetLayout::Ref>& setLayouts);

    // States
    CRenderPass::Ref CreateRenderPass(const CRenderPassDesc& desc);
    CPipeline::Ref CreatePipeline(const CPipelineDesc& desc);
    CPipeline::Ref CreateComputePipeline(const CComputePipelineDesc& desc);
    CSampler::Ref CreateSampler(const CSamplerDesc& desc);

    // Command submission
    CCommandQueue::Ref CreateCommandQueue();
    CCommandQueue::Ref CreateCommandQueue(EQueueType queueType);

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
        return GetQueueFamily(EQueueType::Copy) != GetQueueFamily(EQueueType::Render);
    }
    bool IsComputeQueueSeparate() const
    {
        return GetQueueFamily(EQueueType::Compute) != GetQueueFamily(EQueueType::Render);
    }

    // Getters for global objects
    uint32_t GetQueueFamily(EQueueType t) const { return QueueFamilies[static_cast<int>(t)]; }
    VkQueue GetVkQueue(EQueueType t) const { return Queues[static_cast<int>(t)][0]; }
    VmaAllocator GetAllocator() const { return Allocator; }

    CPersistentMappedRingBuffer* GetHugeConstantBuffer() const { return HugeConstantBuffer.get(); }
    VkPipelineCache GetPipelineCache() const { return PipelineCache; }

    CCommandQueueVk::Ref GetDefaultRenderQueue() const { return DefaultRenderQueue; }
    CCommandQueueVk::Ref GetDefaultCopyQueue() const { return DefaultCopyQueue; }

    void AddPostFrameCleanup(std::function<void(CDeviceVk&)> callback);

private:
    VkDevice Device;

    // NOTE: according to some AMD doc https://gpuopen.com/concurrent-execution-asynchronous-queues/
    //   it's best to stick to one queue per family for current GPUs
    VkPhysicalDevice PhysicalDevice;
    VkPhysicalDeviceProperties Properties;

    // Global objects
    uint32_t QueueFamilies[static_cast<int>(EQueueType::Count)];
    std::vector<VkQueue> Queues[static_cast<int>(EQueueType::Count)];
    VmaAllocator Allocator;
    std::unique_ptr<CPersistentMappedRingBuffer> HugeConstantBuffer;
    VkPipelineCache PipelineCache;
    CCommandQueueVk::Ref DefaultRenderQueue;
    CCommandQueueVk::Ref DefaultCopyQueue;

    friend class CCommandQueueVk; // Allow queues to grab cleanup functors
    std::mutex DeviceMutex;
    std::vector<std::function<void(CDeviceVk&)>> PostFrameCleanup;
};

} /* namespace RHI */
