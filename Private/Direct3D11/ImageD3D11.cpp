#include "ImageD3D11.h"
#include "DeviceD3D11.h"
#include "ConstantConverter.h"
#include "RHIException.h"

namespace RHI
{

CImageD3D11::CImageD3D11(CDeviceD3D11* parent, void* objPtr, uint32_t dimensions)
    : Parent(parent), TexObjPtr(objPtr), Dimensions(dimensions)
{
}

CImageD3D11::~CImageD3D11()
{
    switch (Dimensions)
    {
    case 1:
        AsTexture1D()->Release();
        break;
    case 2:
        AsTexture2D()->Release();
        break;
    case 3:
        AsTexture3D()->Release();
        break;
    default:
        break;
    }
}

void CImageD3D11::CreateFromMem(const void* mem)
{
    if (!bIsDescOnly)
    {
        throw CRHIException("CImageD3D11::CreateFromMem called when the texture object is already created");
    }

    //TODO: handle not just 2D textures

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = mem;
    initData.SysMemPitch = Desc2D.Width * 4; //TODO: not always multiply by 4
    initData.SysMemSlicePitch = 0;
    HRESULT hr;
    if (mem)
        hr = Parent->D3dDevice->CreateTexture2D(&Desc2D, &initData, (ID3D11Texture2D**)&TexObjPtr);
    else
        hr = Parent->D3dDevice->CreateTexture2D(&Desc2D, nullptr, (ID3D11Texture2D**)&TexObjPtr);
    if (!SUCCEEDED(hr))
        throw CRHIRuntimeError("Could not lazily create texture.");

    bIsDescOnly = false;
}

CImageViewD3D11::CImageViewD3D11(CDeviceD3D11* parent, CImageD3D11* image, const CImageViewDesc& desc)
    : Parent(parent), Image(image), Desc(desc)
{
}

ComPtr<ID3D11ShaderResourceView> CImageViewD3D11::GetShaderResourceView() const
{
    if (!SRV)
    {
        auto desc = ConvertDescToSRV(Desc);
        HRESULT hr = Parent->D3dDevice->CreateShaderResourceView(Image->AsD3D11Resource(), &desc, SRV.GetAddressOf());
        if (!SUCCEEDED(hr))
            throw CRHIRuntimeError("Could not GetShaderResourceView: failed to CreateShaderResourceView");
    }
    return SRV;
}

ComPtr<ID3D11RenderTargetView> CImageViewD3D11::GetRenderTargetView() const
{
    if (!RTV)
    {
        auto desc = ConvertDescToRTV(Desc);
        HRESULT hr = Parent->D3dDevice->CreateRenderTargetView(Image->AsD3D11Resource(), &desc, RTV.GetAddressOf());
        if (!SUCCEEDED(hr))
            throw CRHIRuntimeError("Could not GetRenderTargetView: failed to CreateRenderTargetView");
    }
    return RTV;
}

ComPtr<ID3D11DepthStencilView> CImageViewD3D11::GetDepthStencilView() const
{
    if (!DSV)
    {
        auto desc = ConvertDescToDSV(Desc);
        HRESULT hr = Parent->D3dDevice->CreateDepthStencilView(Image->AsD3D11Resource(), &desc, DSV.GetAddressOf());
        if (!SUCCEEDED(hr))
            throw CRHIRuntimeError("Could not GetDepthStencilView: failed to CreateDepthStencilView");
    }
    return DSV;
}

} /* namespace RHI */
