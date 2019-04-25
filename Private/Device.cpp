#include "Device.h"
#ifdef RHI_IMPL_DIRECT3D11
#include "Direct3D11/DeviceD3D11.h"
#elif defined(RHI_IMPL_VULKAN)
#include "Vulkan/DeviceVk.h"
#endif

namespace RHI
{

template <typename TDerived>
CBuffer::Ref CDeviceBase<TDerived>::CreateBuffer(size_t size, EBufferUsageFlags usage,
                                                 const void* initialData)
{
    return static_cast<TDerived*>(this)->CreateBuffer(size, usage, initialData);
}

template <typename TDerived>
CImage::Ref CDeviceBase<TDerived>::CreateImage1D(EFormat format, EImageUsageFlags usage,
                                                 uint32_t width, uint32_t mipLevels,
                                                 uint32_t arrayLayers, uint32_t sampleCount,
                                                 const void* initialData)
{
    return static_cast<TDerived*>(this)->CreateImage1D(format, usage, width, mipLevels, arrayLayers,
                                                       sampleCount, initialData);
}

template <typename TDerived>
CImage::Ref CDeviceBase<TDerived>::CreateImage2D(EFormat format, EImageUsageFlags usage,
                                                 uint32_t width, uint32_t height,
                                                 uint32_t mipLevels, uint32_t arrayLayers,
                                                 uint32_t sampleCount, const void* initialData)
{
    return static_cast<TDerived*>(this)->CreateImage2D(format, usage, width, height, mipLevels,
                                                       arrayLayers, sampleCount, initialData);
}

template <typename TDerived>
CImage::Ref CDeviceBase<TDerived>::CreateImage3D(EFormat format, EImageUsageFlags usage,
                                                 uint32_t width, uint32_t height, uint32_t depth,
                                                 uint32_t mipLevels, uint32_t arrayLayers,
                                                 uint32_t sampleCount, const void* initialData)
{
    return static_cast<TDerived*>(this)->CreateImage3D(
        format, usage, width, height, depth, mipLevels, arrayLayers, sampleCount, initialData);
}

template <typename TDerived>
CImageView::Ref CDeviceBase<TDerived>::CreateImageView(const CImageViewDesc& desc,
                                                       CImage::Ref image)
{
    return static_cast<TDerived*>(this)->CreateImageView(desc, image);
}

template <typename TDerived>
CShaderModule::Ref CDeviceBase<TDerived>::CreateShaderModule(size_t size, const void* pCode)
{
    return static_cast<TDerived*>(this)->CreateShaderModule(size, pCode);
}

template <typename TDerived>
CRenderPass::Ref CDeviceBase<TDerived>::CreateRenderPass(const CRenderPassDesc& desc)
{
    return static_cast<TDerived*>(this)->CreateRenderPass(desc);
}

template <typename TDerived>
CPipeline::Ref CDeviceBase<TDerived>::CreatePipeline(const CPipelineDesc& desc)
{
    return static_cast<TDerived*>(this)->CreatePipeline(desc);
}

template <typename TDerived>
CSampler::Ref CDeviceBase<TDerived>::CreateSampler(const CSamplerDesc& desc)
{
    return static_cast<TDerived*>(this)->CreateSampler(desc);
}

template <typename TDerived> IImmediateContext::Ref CDeviceBase<TDerived>::GetImmediateContext()
{
    return static_cast<TDerived*>(this)->GetImmediateContext();
}

template <typename TDerived> CCommandList::Ref CDeviceBase<TDerived>::CreateCommandList()
{
    return static_cast<TDerived*>(this)->CreateCommandList();
}

template <typename TDerived>
ICopyContext::Ref CDeviceBase<TDerived>::CreateCopyContext(CCommandList& cmdList)
{
    return static_cast<TDerived*>(this)->CreateCopyContext(cmdList);
}

template <typename TDerived>
IComputeContext::Ref CDeviceBase<TDerived>::CreateComputeContext(CCommandList& cmdList)
{
    return static_cast<TDerived*>(this)->CreateComputeContext(cmdList);
}

template <typename TDerived>
IRenderPassContext::Ref
CDeviceBase<TDerived>::CreateRenderPassContext(CCommandList& cmdList, CRenderPass& renderPass,
                                               const std::vector<CClearValue>& clearValues)
{
    return static_cast<TDerived*>(this)->CreateRenderPassContext(cmdList, renderPass, clearValues);
}

template <typename TDerived>
CSwapChain::Ref CDeviceBase<TDerived>::CreateSwapChain(const CPresentationSurfaceDesc& info,
                                                       EFormat format)
{
    return static_cast<TDerived*>(this)->CreateSwapChain(info, format);
}

template <typename TDerived> void CDeviceBase<TDerived>::WaitIdle()
{
    return static_cast<TDerived*>(this)->WaitIdle();
}

// Explicitly instanciate the wrapper for the chosen implementation
template class RHI_API CDeviceBase<TChooseImpl<CDeviceBase>::TDerived>;

} /* namespace RHI */
