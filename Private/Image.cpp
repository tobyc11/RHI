#include "Image.h"
#ifdef RHI_IMPL_DIRECT3D11
#include "Direct3D11/ImageD3D11.h"
#endif

namespace Nome::RHI
{

template<typename TDerived>
CImageBase<TDerived>::CImageBase()
{
}

//Explicitly instanciate the wrapper for the chosen implementation
template class RHI_API CImageBase<TChooseImpl<CImageBase>::TDerived>;

} /* namespace Nome::RHI */
