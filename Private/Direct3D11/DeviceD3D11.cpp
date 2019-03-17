#include "DeviceD3D11.h"
#include "ImageD3D11.h"
#include "BufferD3D11.h"
#include "SamplerD3D11.h"
#include "SwapChainD3D11.h"
#include "CommandListD3D11.h"
#include "StateCacheD3D11.h"
#include "ShaderD3D11.h"
#include "ConstantConverter.h"
#include "AbstractionBreaker.h"
#include "RenderGraph.h"
#include "RHIException.h"

namespace RHI
{

//Part of the AbstractionBreaker, this just seems like a good place for implementation
CNativeDevice GetNativeDevice(CDevice* device)
{
    CNativeDevice native;
    auto* deviceImpl = static_cast<CDeviceD3D11*>(device);
    native.D3dDevice = deviceImpl->D3dDevice;
    native.ImmediateContext = deviceImpl->ImmediateContext;
    return native;
}

void CNativeRenderPass::BindRenderTargets() const
{
    ComPtr<ID3D11DepthStencilView> DSV;
    CNodeId depthId = GetDepthStencilAttachment();
    if (depthId != kInvalidNodeId)
    {
        auto& attachment = GetRenderGraph().GetRenderTarget(depthId);
        auto imageView = attachment.GetImageView();
        auto* imageViewImpl = static_cast<CImageViewD3D11*>(imageView.Get());
        DSV = imageViewImpl->GetDepthStencilView();
    }

    std::vector<ID3D11RenderTargetView*> RTVs;
    ForEachColorAttachment([&](CNodeId id)
    {
        if (id == kInvalidNodeId)
            return;
        auto& attachment = GetRenderGraph().GetRenderTarget(id);
        auto imageView = attachment.GetImageView();
        auto* imageViewImpl = static_cast<CImageViewD3D11*>(imageView.Get());
        auto rtv = imageViewImpl->GetRenderTargetView();
        RTVs.push_back(rtv.Get());
    });

    ImmediateContext->OMSetRenderTargets((UINT)RTVs.size(), RTVs.data(), DSV.Get());
}

CDeviceD3D11::CDeviceD3D11(EDeviceCreateHints hints)
{
    //Doesn't respect the hints yet

    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        DriverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice(nullptr, DriverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
            D3D11_SDK_VERSION, D3dDevice.GetAddressOf(), &FeatureLevel, ImmediateContext.GetAddressOf());

        if (hr == E_INVALIDARG)
        {
            // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
            hr = D3D11CreateDevice(nullptr, DriverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                D3D11_SDK_VERSION, D3dDevice.GetAddressOf(), &FeatureLevel, ImmediateContext.GetAddressOf());
        }

        if (SUCCEEDED(hr))
            break;
    }
    if (FAILED(hr))
        throw CRHIException("Device creation failed");

    StateCache.reset(new CStateCacheD3D11(this));
}

CDeviceD3D11::~CDeviceD3D11()
{
#ifdef _DEBUG
    ID3D11Debug* debugDevice = nullptr;
    HRESULT hr = D3dDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debugDevice));
    if (SUCCEEDED(hr))
    {
        debugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
        if (debugDevice)
            debugDevice->Release();
    }
#endif
}

sp<CBuffer> CDeviceD3D11::CreateBuffer(uint32_t size, EBufferUsageFlags usage, const void* initialData)
{
    return new CBufferD3D11(D3dDevice.Get(), size, usage, initialData);
}

sp<CImage> CDeviceD3D11::CreateImage1D(EFormat format, EImageUsageFlags usage, uint32_t width, uint32_t mipLevels, uint32_t arrayLayers)
{
    throw std::runtime_error("unimplemented");
}

sp<CImage> CDeviceD3D11::CreateImage2D(EFormat format, EImageUsageFlags usage, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t arrayLayers)
{
    //Determine various properties
    UINT bindFlags = 0;
    bool bCreateImmediately = false;
    if (Any(usage, EImageUsageFlags::DepthStencil))
    {
        bCreateImmediately = true;
        bindFlags |= D3D11_BIND_DEPTH_STENCIL;
    }

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = mipLevels;
    desc.ArraySize = arrayLayers;
    desc.Format = Convert(format);
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = bindFlags;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    sp<CImageD3D11> image = new CImageD3D11(this, desc);
    if (bCreateImmediately)
        image->CreateFromMem(nullptr);
    return image;
}

sp<CImage> CDeviceD3D11::CreateImage3D(EFormat format, EImageUsageFlags usage, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers)
{
    throw std::runtime_error("unimplemented");
}

sp<CImageView> CDeviceD3D11::CreateImageView(const CImageViewDesc& desc, CImage* image)
{
    auto* imageImpl = static_cast<CImageD3D11*>(image);
    return new CImageViewD3D11(this, imageImpl, desc);
}

sp<CSampler> CDeviceD3D11::CreateSampler(const CSamplerDesc& desc)
{
    //D3D11_FILTER Min Mag Mip
    //              00  00  00
    //  0 means point and 1 means linear
    uint32_t filter = 0;
    if (desc.AnisotropyEnable)
        filter = D3D11_FILTER_ANISOTROPIC;
    else
    {
        filter |= static_cast<uint32_t>(desc.MinFilter) << 4;
        filter |= static_cast<uint32_t>(desc.MagFilter) << 2;
        filter |= static_cast<uint32_t>(desc.MipmapMode) << 0;
    }

    ID3D11SamplerState* sampler;
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER(filter);
    sampDesc.AddressU = Convert(desc.AddressModeU);
    sampDesc.AddressV = Convert(desc.AddressModeV);
    sampDesc.AddressW = Convert(desc.AddressModeW);
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = desc.MinLod;
    sampDesc.MaxLOD = desc.MaxLod;

    HRESULT hr;
    hr = D3dDevice->CreateSamplerState(&sampDesc, &sampler);
    if (FAILED(hr))
        throw CRHIRuntimeError("Could not create sampler.");

    sp<CSampler> result = new CSamplerD3D11(sampler);
    sampler->Release();
    return result;
}

sp<CSwapChain> CDeviceD3D11::CreateSwapChain(const CSwapChainCreateInfo& info)
{
    HRESULT hr = S_OK;
    IDXGISwapChain* pSwapChain = nullptr;
    IDXGISwapChain1* pSwapChain1 = nullptr;

    HWND hWnd = (HWND)info.OSWindowHandle;
    RECT rc;
    GetClientRect(hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
    IDXGIFactory1* dxgiFactory = nullptr;
    {
        IDXGIDevice* dxgiDevice = nullptr;
        hr = D3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
        if (SUCCEEDED(hr))
        {
            IDXGIAdapter* adapter = nullptr;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr))
            {
                hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
                adapter->Release();
            }
            dxgiDevice->Release();
        }
    }
    if (FAILED(hr))
        throw CRHIException("Device creation failed: couldn't obtain DXGI factory");

    // Create swap chain
    IDXGIFactory2* dxgiFactory2 = nullptr;
    hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
    if (dxgiFactory2)
    {
        // DirectX 11.1 or later
        hr = D3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(D3dDevice1.GetAddressOf()));
        if (SUCCEEDED(hr))
        {
            (void)ImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(ImmediateContext1.GetAddressOf()));
        }

        DXGI_SWAP_CHAIN_DESC1 sd = {};
        sd.Width = width;
        sd.Height = height;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;

        hr = dxgiFactory2->CreateSwapChainForHwnd(D3dDevice.Get(), hWnd, &sd, nullptr, nullptr, &pSwapChain1);
        if (SUCCEEDED(hr))
        {
            hr = pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&pSwapChain));
        }

        dxgiFactory2->Release();
    }
    else
    {
        // DirectX 11.0 systems
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 1;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;

        hr = dxgiFactory->CreateSwapChain(D3dDevice.Get(), &sd, &pSwapChain);
    }

    // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
    dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

    dxgiFactory->Release();

    if (FAILED(hr))
        throw CRHIException("CreateSwapChain failed");

    sp<CSwapChain> result = new CSwapChainD3D11(pSwapChain);
    if (pSwapChain1) pSwapChain1->Release();
    return result;
}

CCommandListD3D11* CDeviceD3D11::CreateCommandList()
{
    return new CCommandListD3D11(this);
}

} /* namespace RHI */
