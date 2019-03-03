#include "ImageD3D11.h"

namespace Nome::RHI
{

CImageD3D11::CImageD3D11(void* objPtr, uint32_t dimensions) : TexObjPtr(objPtr), Dimensions(dimensions)
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

} /* namespace Nome::RHI */
