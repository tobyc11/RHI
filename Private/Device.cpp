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
CFramebuffer::Ref CDeviceBase<TDerived>::CreateFramebuffer(const CFramebufferDesc& desc)
{
    return static_cast<TDerived*>(this)->CreateFramebuffer(desc);
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

template <typename TDerived> IRenderContext::Ref CDeviceBase<TDerived>::GetImmediateContext()
{
    return static_cast<TDerived*>(this)->GetImmediateContext();
}

template <typename TDerived> IRenderContext::Ref CDeviceBase<TDerived>::CreateDeferredContext()
{
    return static_cast<TDerived*>(this)->CreateDeferredContext();
}

// Explicitly instanciate the wrapper for the chosen implementation
template class RHI_API CDeviceBase<TChooseImpl<CDeviceBase>::TDerived>;

} /* namespace RHI */
