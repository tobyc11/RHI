#pragma once
#include "CommandQueue.h"
#include "DescriptorSet.h"
#include "ManagedPipeline.h"
#include "Pipeline.h"
#include "RHIException.h"
#include "RenderContext.h"
#include "RenderPass.h"
#include "Resources.h"
#include "Sampler.h"
#include "ShaderModule.h"
#include "SwapChain.h"
#include <LangUtils.h>

namespace RHI
{

struct CPresentationSurfaceDesc;

enum class EDeviceCreateHints
{
    NoHint,
    Integrated,
    Discrete,
};

template <typename TDerived> class RHI_API CDeviceBase : public tc::FNonCopyable
{
public:
    typedef std::shared_ptr<CDeviceBase> Ref;

    virtual ~CDeviceBase() = default;

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
    CDescriptorSetLayout::Ref CreateDescriptorSetLayout(const std::vector<CDescriptorSetLayoutBinding>& bindings);
    CPipelineLayout::Ref CreatePipelineLayout(const std::vector<CDescriptorSetLayout::Ref>& setLayouts);

    // States
    CRenderPass::Ref CreateRenderPass(const CRenderPassDesc& desc);
    CPipeline::Ref CreatePipeline(const CPipelineDesc& desc);
    CManagedPipeline::Ref CreateManagedPipeline(CPipelineDesc& desc);
    CSampler::Ref CreateSampler(const CSamplerDesc& desc);

    // Command submission
    CCommandQueue::Ref CreateCommandQueue();

    // Windowing system interface
    CSwapChain::Ref CreateSwapChain(const CPresentationSurfaceDesc& info, EFormat format);

    void WaitIdle();

protected:
    CDeviceBase() = default;
};

} /* namespace RHI */
