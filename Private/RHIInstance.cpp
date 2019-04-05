#include "RHIInstance.h"
#ifdef RHI_IMPL_DIRECT3D11
#include "Direct3D11/DeviceD3D11.h"
#elif defined(RHI_IMPL_VULKAN)
#include "Vulkan/DeviceVk.h"
#endif

namespace RHI
{

extern void InitRHIInstance();
extern void ShutdownRHIInstance();

static thread_local CDevice::Ref CurrDevice;

CInstance& CInstance::Get()
{
    static CInstance globalSingleton;
    return globalSingleton;
}

CDevice::Ref CInstance::GetCurrDevice() const { return CurrDevice; }

void CInstance::SetCurrDevice(CDevice::Ref device) { CurrDevice = device; }

CDevice::Ref CInstance::CreateDevice(EDeviceCreateHints hints)
{
    return std::make_shared<TChooseImpl<CDeviceBase>::TDerived>(hints);
}

CInstance::CInstance() { InitRHIInstance(); }

CInstance::~CInstance() { ShutdownRHIInstance(); }

} /* namespace RHI */
