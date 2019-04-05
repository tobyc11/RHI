#pragma once
#include "Image.h"
#include "ImageView.h"
#include "D3D11Platform.h"
#include "RHIException.h"

#include <variant>

namespace RHI
{


class CImageD3D11 : public CImageBase<CImageD3D11>
{
public:
    CImageD3D11(CDeviceD3D11& parent, ComPtr<ID3D11Texture1D> p);
    CImageD3D11(CDeviceD3D11& parent, ComPtr<ID3D11Texture2D> p);
    CImageD3D11(CDeviceD3D11& parent, ComPtr<ID3D11Texture3D> p);
    CImageD3D11(CDeviceD3D11& parent, D3D11_TEXTURE1D_DESC desc) : Parent(parent), Dimensions(1), Desc1D(desc), bIsDescOnly(true) {}
    CImageD3D11(CDeviceD3D11& parent, D3D11_TEXTURE2D_DESC desc) : Parent(parent), Dimensions(2), Desc2D(desc), bIsDescOnly(true) {}
    CImageD3D11(CDeviceD3D11& parent, D3D11_TEXTURE3D_DESC desc) : Parent(parent), Dimensions(3), Desc3D(desc), bIsDescOnly(true) {}
    ~CImageD3D11();

    CImageD3D11(const CImageD3D11&) = delete;
    CImageD3D11(CImageD3D11&&) = delete;
    CImageD3D11& operator=(const CImageD3D11&) = delete;
    CImageD3D11& operator=(CImageD3D11&&) = delete;

    void CreateFromMem(const void* mem) const;
    void CopyFrom(const void* mem) { CreateFromMem(mem); }

    ID3D11Texture1D* AsTexture1D() const;
    ID3D11Texture2D* AsTexture2D() const;
    ID3D11Texture3D* AsTexture3D() const;

    ID3D11Resource* AsD3D11Resource() const
    {
        if (Dimensions == 1)
            return AsTexture1D();
        if (Dimensions == 2)
            return AsTexture2D();
        if (Dimensions == 3)
            return AsTexture3D();
        throw CRHIRuntimeError("AsD3D11Resource failed");
    }

private:
    CDeviceD3D11& Parent;

    using ID3DTextureAny = std::variant<ComPtr<ID3D11Texture1D>, ComPtr<ID3D11Texture2D>, ComPtr<ID3D11Texture3D>>;
    
    uint32_t Dimensions;
    mutable ID3DTextureAny TexturePtr;
    union
    {
        D3D11_TEXTURE1D_DESC Desc1D;
        D3D11_TEXTURE2D_DESC Desc2D;
        D3D11_TEXTURE3D_DESC Desc3D;
    };
    mutable bool bIsDescOnly = false;
};

class CImageViewD3D11 : public CImageView
{
public:
    CImageViewD3D11(CDeviceD3D11& parent, CImageD3D11* image, const CImageViewDesc& desc);

    ComPtr<ID3D11ShaderResourceView> GetShaderResourceView() const;
    ComPtr<ID3D11RenderTargetView> GetRenderTargetView() const;
    ComPtr<ID3D11DepthStencilView> GetDepthStencilView() const;

private:
    CDeviceD3D11& Parent;
    CImageD3D11::Ref Image;
    CImageViewDesc Desc;

    mutable ComPtr<ID3D11ShaderResourceView> SRV;
    mutable ComPtr<ID3D11RenderTargetView> RTV;
    mutable ComPtr<ID3D11DepthStencilView> DSV;
};

} /* namespace RHI */
