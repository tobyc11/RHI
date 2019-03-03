#pragma once
#include "Image.h"
#include "ImageView.h"
#include "D3D11Platform.h"

namespace Nome::RHI
{

class CImageD3D11 : public CImageBase<CImageD3D11>
{
public:
    CImageD3D11(void* objPtr, uint32_t dimensions);
    ~CImageD3D11();

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

private:
    uint32_t Dimensions;
    void* TexObjPtr = nullptr;
};

class CImageViewD3D11 : public CImageView
{
public:
};

} /* namespace Nome::RHI */
