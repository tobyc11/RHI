#pragma once
#include "Image.h"
#include "ImageView.h"
#include "Sampler.h"
#include "Buffer.h"
#include "SwapChain.h"
#include <RefBase.h>

namespace RHI
{

using tc::sp;

enum class EDeviceCreateHints
{
    NoHint,
    Integrated,
    Discrete,
};

template <typename TDerived>
class CDeviceBase : public tc::TLightRefBase<CDeviceBase<TDerived>>
{
protected:
    CDeviceBase();

public:
    virtual ~CDeviceBase() = default;

    sp<CBuffer> CreateBuffer(uint32_t size, EBufferUsageFlags usage, const void* initialData = nullptr);

    sp<CImage> CreateImage1D(EFormat format, EImageUsageFlags usage, uint32_t width, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);
    sp<CImage> CreateImage2D(EFormat format, EImageUsageFlags usage, uint32_t width, uint32_t height, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);
    sp<CImage> CreateImage3D(EFormat format, EImageUsageFlags usage, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);
    sp<CImageView> CreateImageView(const CImageViewDesc& desc, CImage* image);

    sp<CSampler> CreateSampler(const CSamplerDesc& desc);

    sp<CSwapChain> CreateSwapChain(const CSwapChainCreateInfo& info);
};

using CDevice = TChooseImpl<CDeviceBase>::TConcreteBase;

} /* namespace RHI */
