#pragma once
#include "Device.h"
#include "D3D11Platform.h"

namespace Nome::RHI
{

class CDeviceD3D11 : public CDeviceBase<CDeviceD3D11>
{
public:
    CDeviceD3D11(EDeviceCreateHints hints);

    sp<CBuffer> CreateBuffer(uint32_t size, EBufferUsageFlags usage, void* initialData = nullptr);

    sp<CImage> CreateImage1D(EFormat format, EImageUsageFlags usage, uint32_t width, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);
    sp<CImage> CreateImage2D(EFormat format, EImageUsageFlags usage, uint32_t width, uint32_t height, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);
    sp<CImage> CreateImage3D(EFormat format, EImageUsageFlags usage, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);

    sp<CSampler> CreateSampler(const CSamplerDesc& desc);

    sp<CSwapChain> CreateSwapChain(const CSwapChainCreateInfo& info);

private:
    D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_11_0;
    ComPtr<ID3D11Device> D3dDevice;
    ComPtr<ID3D11Device1> D3dDevice1;
    ComPtr<ID3D11DeviceContext> ImmediateContext;
    ComPtr<ID3D11DeviceContext1> ImmediateContext1;
};

} /* namespace Nome::RHI */
