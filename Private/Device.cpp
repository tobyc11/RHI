#include "Device.h"
#ifdef RHI_IMPL_DIRECT3D11
#include "Direct3D11/DeviceD3D11.h"
#endif

namespace Nome::RHI
{

template<typename TDerived>
inline CDeviceBase<TDerived>::CDeviceBase()
{
}

//Explicitly instanciate the wrapper for the chosen implementation
template class RHI_API CDeviceBase<TChooseImpl<CDeviceBase>::TDerived>;

} /* namespace Nome::RHI */
