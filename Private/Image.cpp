#include "Image.h"
#ifdef RHI_IMPL_DIRECT3D11
#include "Direct3D11/ImageD3D11.h"
#endif

namespace RHI
{

template<typename TDerived>
CImageBase<TDerived>::CImageBase()
{
}

template<typename TDerived>
void CImageBase<TDerived>::CopyFrom(const void* mem)
{
    static_cast<TDerived*>(this)->CopyFrom(mem);
}

//Explicitly instanciate the wrapper for the chosen implementation
template class RHI_API CImageBase<TChooseImpl<CImageBase>::TDerived>;

} /* namespace RHI */
