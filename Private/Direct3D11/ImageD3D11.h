#pragma once
#include "Image.h"
#include "ImageView.h"
#include "D3D11Platform.h"

namespace Nome::RHI
{

class CImageD3D11 : public CImageBase<CImageD3D11>
{
public:
    CImageD3D11(CDeviceD3D11* parent, void* objPtr, uint32_t dimensions);
    CImageD3D11(CDeviceD3D11* parent, D3D11_TEXTURE1D_DESC desc) : Parent(parent), Desc1D(desc), bIsDescOnly(true) {}
    CImageD3D11(CDeviceD3D11* parent, D3D11_TEXTURE2D_DESC desc) : Parent(parent), Desc2D(desc), bIsDescOnly(true) {}
    CImageD3D11(CDeviceD3D11* parent, D3D11_TEXTURE3D_DESC desc) : Parent(parent), Desc3D(desc), bIsDescOnly(true) {}
    ~CImageD3D11();

    void CopyFrom(void* mem);

    ID3D11Texture1D* AsTexture1D() const
    {
        if (Dimensions == 1)
            return static_cast<ID3D11Texture1D*>(TexObjPtr);
        return nullptr;
    }
    
    ID3D11Texture2D* AsTexture2D() const
    {
        if (Dimensions == 2)
            return static_cast<ID3D11Texture2D*>(TexObjPtr);
        return nullptr;
    }

    ID3D11Texture3D* AsTexture3D() const
    {
        if (Dimensions == 3)
            return static_cast<ID3D11Texture3D*>(TexObjPtr);
        return nullptr;
    }

    void* AsVoidPtr() const { return TexObjPtr; }

private:
    CDeviceD3D11* Parent;

    uint32_t Dimensions;
    void* TexObjPtr = nullptr;
    union
    {
        D3D11_TEXTURE1D_DESC Desc1D;
        D3D11_TEXTURE2D_DESC Desc2D;
        D3D11_TEXTURE3D_DESC Desc3D;
    };
    bool bIsDescOnly = false;
};

class CImageViewD3D11 : public CImageView
{
public:
    CImageViewD3D11(ID3D11ShaderResourceView* ptr) : SRVPtr(ptr) {}

private:
    ID3D11ShaderResourceView* SRVPtr;
};

} /* namespace Nome::RHI */
