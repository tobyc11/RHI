#pragma once
#include "Device.h"

namespace RHI
{

// The header file is named RHIInstance to avoid name conflict with windows sdk
// There is only one global instance shared across all threads
class RHI_API CInstance
{
public:
    static CInstance& Get();

    CDevice::Ref GetCurrDevice() const;
    void SetCurrDevice(CDevice::Ref device);

    CDevice::Ref CreateDevice(EDeviceCreateHints hints);

private:
    CInstance();
    CInstance(const CInstance&) = delete;
    CInstance(CInstance&&) = delete;
    CInstance& operator=(const CInstance&) = delete;
    CInstance& operator=(CInstance&&) = delete;
    ~CInstance();
};

} /* namespace RHI */
