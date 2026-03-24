#pragma once
#include "Device.h"
#include "MtlCommon.h"

namespace RHI
{

class CDeviceMetal : public CDevice
{
public:
    typedef std::shared_ptr<CDeviceMetal> Ref;

    explicit CDeviceMetal(EDeviceCreateHints hints);
    ~CDeviceMetal() override;

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

    CShaderModule::Ref CreateShaderModule(size_t size, const void* pCode);
    CDescriptorSetLayout::Ref
    CreateDescriptorSetLayout(const std::vector<CDescriptorSetLayoutBinding>& bindings);
    CPipelineLayout::Ref
    CreatePipelineLayout(const std::vector<CDescriptorSetLayout::Ref>& setLayouts);

    CRenderPass::Ref CreateRenderPass(const CRenderPassDesc& desc);
    CPipeline::Ref CreatePipeline(const CPipelineDesc& desc);
    CPipeline::Ref CreateComputePipeline(const CComputePipelineDesc& desc);
    CSampler::Ref CreateSampler(const CSamplerDesc& desc);

    CCommandQueue::Ref CreateCommandQueue();
    CSwapChain::Ref CreateSwapChain(const CPresentationSurfaceDesc& info, EFormat format);
    void WaitIdle();

    id GetMTLDevice() const { return Device; }
    id GetDefaultQueue() const { return DefaultQueue; }

    void AddPostFrameCleanup(std::function<void(CDeviceMetal&)> callback);

private:
    CImage::Ref InternalCreateImage(unsigned long type, EFormat format, EImageUsageFlags usage,
                                    uint32_t width, uint32_t height, uint32_t depth,
                                    uint32_t mipLevels, uint32_t arrayLayers, uint32_t sampleCount,
                                    const void* initialData);

    id Device;
    id DefaultQueue;

    friend class CCommandQueueMetal;
    friend class CCommandListMetal;
    std::mutex DeviceMutex;
    std::vector<std::function<void(CDeviceMetal&)>> PostFrameCleanup;
};

} /* namespace RHI */
