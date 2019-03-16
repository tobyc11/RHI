#include "Device.h"
#ifdef RHI_IMPL_DIRECT3D11
#include "Direct3D11/DeviceD3D11.h"
#endif

namespace RHI
{

template<typename TDerived>
CDeviceBase<TDerived>::CDeviceBase()
{
}

template<typename TDerived>
sp<CBuffer> CDeviceBase<TDerived>::CreateBuffer(uint32_t size, EBufferUsageFlags usage, const void* initialData)
{
    return static_cast<TDerived*>(this)->CreateBuffer(size, usage, initialData);
}

template<typename TDerived>
sp<CImage> CDeviceBase<TDerived>::CreateImage1D(EFormat format, EImageUsageFlags usage, uint32_t width, uint32_t mipLevels, uint32_t arrayLayers)
{
    return static_cast<TDerived*>(this)->CreateImage1D(format, usage, width, mipLevels, arrayLayers);
}

template<typename TDerived>
sp<CImage> CDeviceBase<TDerived>::CreateImage2D(EFormat format, EImageUsageFlags usage, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t arrayLayers)
{
    return static_cast<TDerived*>(this)->CreateImage2D(format, usage, width, height, mipLevels, arrayLayers);
}

template<typename TDerived>
sp<CImage> CDeviceBase<TDerived>::CreateImage3D(EFormat format, EImageUsageFlags usage, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers)
{
    return static_cast<TDerived*>(this)->CreateImage3D(format, usage, width, height, depth, mipLevels, arrayLayers);
}

template<typename TDerived>
sp<CImageView> CDeviceBase<TDerived>::CreateImageView(const CImageViewDesc& desc, CImage* image)
{
    return static_cast<TDerived*>(this)->CreateImageView(desc, image);
}

template<typename TDerived>
sp<CSampler> CDeviceBase<TDerived>::CreateSampler(const CSamplerDesc & desc)
{
    return static_cast<TDerived*>(this)->CreateSampler(desc);
}

template<typename TDerived>
sp<CSwapChain> CDeviceBase<TDerived>::CreateSwapChain(const CSwapChainCreateInfo & info)
{
    return static_cast<TDerived*>(this)->CreateSwapChain(info);
}

//Explicitly instanciate the wrapper for the chosen implementation
template class RHI_API CDeviceBase<TChooseImpl<CDeviceBase>::TDerived>;

} /* namespace RHI */
