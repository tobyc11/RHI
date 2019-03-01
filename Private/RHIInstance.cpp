#include "RHIInstance.h"
#ifdef RHI_IMPL_DIRECT3D11
#include "Direct3D11/DeviceD3D11.h"
#endif

namespace Nome::RHI
{

static thread_local CDevice* CurrDevice;

CInstance& CInstance::Get()
{
    static CInstance globalSingleton;
    return globalSingleton;
}

CDevice* CInstance::GetCurrDevice() const
{
    return CurrDevice;
}

void CInstance::SetCurrDevice(CDevice* device)
{
    CurrDevice = device;
}

sp<CDevice> CInstance::CreateDevice(EDeviceCreateHints hints)
{
    return new TChooseImpl<CDeviceBase>::TDerived(hints);
}

} /* namespace Nome::RHI */
