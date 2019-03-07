#include "ImageD3D11.h"
#include "DeviceD3D11.h"
#include "RHIException.h"

namespace Nome::RHI
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

void CImageD3D11::CopyFrom(void* mem)
{
    if (bIsDescOnly)
    {
        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = mem;
        initData.SysMemPitch = Desc2D.Width * 4;
        initData.SysMemSlicePitch = 0;
        HRESULT hr = Parent->D3dDevice->CreateTexture2D(&Desc2D, &initData, (ID3D11Texture2D**)&TexObjPtr);
        if (!SUCCEEDED(hr))
            throw CRHIRuntimeError("Could not create texture (lazy).");

        bIsDescOnly = false;
    }

    throw std::runtime_error("unimplemented");
}

} /* namespace Nome::RHI */
