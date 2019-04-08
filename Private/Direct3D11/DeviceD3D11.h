#pragma once
#include "D3D11Platform.h"
#include "Device.h"
#include <memory>

namespace RHI
{

class CCommandListD3D11;
class CStateCacheD3D11;
class CShaderCacheD3D11;

class CDeviceD3D11 : public CDeviceBase<CDeviceD3D11>
{
public:
    CDeviceD3D11(EDeviceCreateHints hints);
    ~CDeviceD3D11();

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

    // States
    CRenderPass::Ref CreateRenderPass(const CRenderPassDesc& desc);
    CPipeline::Ref CreatePipeline(const CPipelineDesc& desc);
    CSampler::Ref CreateSampler(const CSamplerDesc& desc);

    // Command submission
    IRenderContext::Ref GetImmediateContext();
    IRenderContext::Ref CreateDeferredContext();

    // Windowing system interface
    CSwapChain::Ref CreateSwapChain(const CPresentationSurfaceDesc& info, EFormat format);

    // Not private for convenience, but make sure not to modify them
    D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_11_0;
    ComPtr<ID3D11Device> D3dDevice;
    ComPtr<ID3D11Device1> D3dDevice1;
    ComPtr<ID3D11DeviceContext> ImmediateContext;
    ComPtr<ID3D11DeviceContext1> ImmediateContext1;

    std::unique_ptr<CStateCacheD3D11> StateCache;
    std::unique_ptr<CShaderCacheD3D11> ShaderCache;
};

} /* namespace RHI */
