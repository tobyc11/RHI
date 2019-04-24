#include "ImageD3D11.h"
#include "ConstantConverter.h"
#include "DeviceD3D11.h"
#include "RHIException.h"

namespace RHI
{

CImageD3D11::CImageD3D11(CDeviceD3D11& parent, ComPtr<ID3D11Texture1D> p)
    : Parent(parent)
    , Dimensions(1)
    , TexturePtr(p)
{
}

CImageD3D11::CImageD3D11(CDeviceD3D11& parent, ComPtr<ID3D11Texture2D> p)
    : Parent(parent)
    , Dimensions(2)
    , TexturePtr(p)
{
}

CImageD3D11::CImageD3D11(CDeviceD3D11& parent, ComPtr<ID3D11Texture3D> p)
    : Parent(parent)
    , Dimensions(3)
    , TexturePtr(p)
{
}

CImageD3D11::~CImageD3D11() {}

void CImageD3D11::CreateFromMem(const void* mem) const
{
    if (!bIsDescOnly)
    {
        throw CRHIException(
            "CImageD3D11::CreateFromMem called when the texture object is already created");
    }

    // TODO: handle not just 2D textures
    ComPtr<ID3D11Texture2D> tex;

    UINT pixelWidth = 0;
    if (Desc2D.Format >= 9 && Desc2D.Format <= 14)
        pixelWidth = 8;
    else if (Desc2D.Format >= 27 && Desc2D.Format <= 47)
        pixelWidth = 4;
    else if (Desc2D.Format >= DXGI_FORMAT_R32G32B32_TYPELESS
             && Desc2D.Format <= DXGI_FORMAT_R32G32B32_SINT)
        pixelWidth = 12;

    assert(pixelWidth != 0);

    bool bWillGenMips = Desc2D.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS;

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = mem;
    initData.SysMemPitch = Desc2D.Width * pixelWidth; // TODO: not always multiply by 4
    initData.SysMemSlicePitch = 0;
    HRESULT hr;
    if (mem && !bWillGenMips)
        hr = Parent.D3dDevice->CreateTexture2D(&Desc2D, &initData, tex.GetAddressOf());
    else
        hr = Parent.D3dDevice->CreateTexture2D(&Desc2D, nullptr, tex.GetAddressOf());
    if (!SUCCEEDED(hr))
        throw CRHIRuntimeError("Could not lazily create texture.");

    if (bWillGenMips && mem)
    {
        Parent.ImmediateContext->UpdateSubresource(tex.Get(), 0, nullptr, mem,
                                                   Desc2D.Width * pixelWidth, 0);

        D3D11_SHADER_RESOURCE_VIEW_DESC srv;
        srv.Format = Desc2D.Format;
        srv.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
        srv.Texture2D.MostDetailedMip = 0;
        srv.Texture2D.MipLevels = static_cast<UINT>(-1);
        ComPtr<ID3D11ShaderResourceView> srvPtr;
        Parent.D3dDevice->CreateShaderResourceView(tex.Get(), &srv, srvPtr.GetAddressOf());
        Parent.ImmediateContext->GenerateMips(srvPtr.Get());
    }

    TexturePtr = tex;
    bIsDescOnly = false;
}

EFormat CImageD3D11::GetFormat() const { throw "unimplemented"; }

EImageUsageFlags CImageD3D11::GetUsageFlags() const { throw "unimplemented"; }

uint32_t CImageD3D11::GetWidth() const { throw "unimplemented"; }

uint32_t CImageD3D11::GetHeight() const { throw "unimplemented"; }

uint32_t CImageD3D11::GetDepth() const { throw "unimplemented"; }

uint32_t CImageD3D11::GetMipLevels() const { throw "unimplemented"; }

uint32_t CImageD3D11::GetArrayLayers() const { throw "unimplemented"; }

uint32_t CImageD3D11::GetSampleCount() const { throw "unimplemented"; }

ID3D11Texture1D* CImageD3D11::AsTexture1D() const
{
    if (bIsDescOnly)
        CreateFromMem(nullptr);
    return std::get<0>(TexturePtr).Get();
}

ID3D11Texture2D* CImageD3D11::AsTexture2D() const
{
    if (bIsDescOnly)
        CreateFromMem(nullptr);
    return std::get<1>(TexturePtr).Get();
}

ID3D11Texture3D* CImageD3D11::AsTexture3D() const
{
    if (bIsDescOnly)
        CreateFromMem(nullptr);
    return std::get<2>(TexturePtr).Get();
}

CImageViewD3D11::CImageViewD3D11(CDeviceD3D11& parent, CImageD3D11::Ref image,
                                 const CImageViewDesc& desc)
    : Parent(parent)
    , Image(image)
    , Desc(desc)
{
}

ComPtr<ID3D11ShaderResourceView> CImageViewD3D11::GetShaderResourceView() const
{
    if (!SRV)
    {
        auto desc = ConvertDescToSRV(Desc);
        HRESULT hr = Parent.D3dDevice->CreateShaderResourceView(Image->AsD3D11Resource(), &desc,
                                                                SRV.GetAddressOf());
        if (!SUCCEEDED(hr))
            throw CRHIRuntimeError(
                "Could not GetShaderResourceView: failed to CreateShaderResourceView");
    }
    return SRV;
}

ComPtr<ID3D11RenderTargetView> CImageViewD3D11::GetRenderTargetView() const
{
    if (!RTV)
    {
        auto desc = ConvertDescToRTV(Desc);
        HRESULT hr = Parent.D3dDevice->CreateRenderTargetView(Image->AsD3D11Resource(), &desc,
                                                              RTV.GetAddressOf());
        if (!SUCCEEDED(hr))
            throw CRHIRuntimeError(
                "Could not GetRenderTargetView: failed to CreateRenderTargetView");
    }
    return RTV;
}

ComPtr<ID3D11DepthStencilView> CImageViewD3D11::GetDepthStencilView() const
{
    if (!DSV)
    {
        auto desc = ConvertDescToDSV(Desc);
        HRESULT hr = Parent.D3dDevice->CreateDepthStencilView(Image->AsD3D11Resource(), &desc,
                                                              DSV.GetAddressOf());
        if (!SUCCEEDED(hr))
            throw CRHIRuntimeError(
                "Could not GetDepthStencilView: failed to CreateDepthStencilView");
    }
    return DSV;
}

} /* namespace RHI */
