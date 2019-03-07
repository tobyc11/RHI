#include "DeviceD3D11.h"
#include "ImageD3D11.h"
#include "BufferD3D11.h"
#include "SamplerD3D11.h"
#include "SwapChainD3D11.h"
#include "CommandListD3D11.h"
#include "StateCacheD3D11.h"
#include "ConstantConverter.h"
#include "RHIException.h"

namespace Nome::RHI
{

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

sp<CBuffer> CDeviceD3D11::CreateBuffer(uint32_t size, EBufferUsageFlags usage, void* initialData)
{
    return new CBufferD3D11(D3dDevice.Get(), size, usage, initialData);
}

sp<CImage> CDeviceD3D11::CreateImage1D(EFormat format, EImageUsageFlags usage, uint32_t width, uint32_t mipLevels, uint32_t arrayLayers)
{
    throw std::runtime_error("unimplemented");
}

sp<CImage> CDeviceD3D11::CreateImage2D(EFormat format, EImageUsageFlags usage, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t arrayLayers)
{
    //Determine bind flags
    UINT bindFlags = 0;
    if (Any(usage, EImageUsageFlags::DepthStencil))
        bindFlags |= D3D11_BIND_DEPTH_STENCIL;

    D3D11_TEXTURE2D_DESC descDepth = {};
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = Convert(format);
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = bindFlags;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    ID3D11Texture2D* result; //don't release, will be released by imaged3d11
    HRESULT hr = D3dDevice->CreateTexture2D(&descDepth, nullptr, &result);
    if (!SUCCEEDED(hr))
        throw CRHIRuntimeError("Could not create texture.");

    sp<CImage> image = new CImageD3D11(this, descDepth);
    return image;
}

sp<CImage> CDeviceD3D11::CreateImage3D(EFormat format, EImageUsageFlags usage, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers)
{
    throw std::runtime_error("unimplemented");
}

sp<CImageView> CDeviceD3D11::CreateImageView(const CImageViewDesc& desc, CImage* image)
{
    ID3D11ShaderResourceView* srv;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = Convert(desc.Format);
    switch (desc.Type)
    {
    case EImageViewType::View1D:
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
        srvDesc.Texture1D.MostDetailedMip = desc.Range.BaseMipLevel;
        srvDesc.Texture1D.MipLevels = desc.Range.LevelCount;
        break;
    case EImageViewType::View1DArray:
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
        srvDesc.Texture1DArray.MostDetailedMip = desc.Range.BaseMipLevel;
        srvDesc.Texture1DArray.MipLevels = desc.Range.LevelCount;
        srvDesc.Texture1DArray.FirstArraySlice = desc.Range.BaseArrayLayer;
        srvDesc.Texture1DArray.ArraySize = desc.Range.LayerCount;
        break;
    case EImageViewType::View2D:
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = desc.Range.BaseMipLevel;
        srvDesc.Texture2D.MipLevels = desc.Range.LevelCount;
        break;
    case EImageViewType::View2DArray:
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        srvDesc.Texture2DArray.MostDetailedMip = desc.Range.BaseMipLevel;
        srvDesc.Texture2DArray.MipLevels = desc.Range.LevelCount;
        srvDesc.Texture2DArray.FirstArraySlice = desc.Range.BaseArrayLayer;
        srvDesc.Texture2DArray.ArraySize = desc.Range.LayerCount;
        break;
    case EImageViewType::View3D:
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
        srvDesc.Texture3D.MostDetailedMip = desc.Range.BaseMipLevel;
        srvDesc.Texture3D.MipLevels = desc.Range.LevelCount;
        break;
    case EImageViewType::Cube:
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MostDetailedMip = desc.Range.BaseMipLevel;
        srvDesc.TextureCube.MipLevels = desc.Range.LevelCount;
        break;
    case EImageViewType::CubeArray:
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
        srvDesc.TextureCubeArray.MostDetailedMip = desc.Range.BaseMipLevel;
        srvDesc.TextureCubeArray.MipLevels = desc.Range.LevelCount;
        srvDesc.TextureCubeArray.First2DArrayFace = desc.Range.BaseArrayLayer;
        srvDesc.TextureCubeArray.NumCubes = desc.Range.LayerCount / 6; //TODO: confirm this
        break;
    default:
        break;
    }

    auto* imageImpl = static_cast<CImageD3D11*>(image);
    HRESULT hr = D3dDevice->CreateShaderResourceView(static_cast<ID3D11Resource*>(imageImpl->AsVoidPtr()), &srvDesc, &srv);
    if (!SUCCEEDED(hr))
        throw CRHIRuntimeError("Could not CreateImageView");

    return new CImageViewD3D11(srv);
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

    return new CSwapChainD3D11(pSwapChain);
}

CCommandListD3D11* CDeviceD3D11::CreateCommandList()
{
    return new CCommandListD3D11(this);
}

} /* namespace Nome::RHI */
