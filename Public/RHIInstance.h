#pragma once
#include "Device.h"

namespace Nome::RHI
{

using tc::sp;

//There is only one global instance shared across all threads
class CInstance
{
public:
    static CInstance& Get();

    CDevice* GetCurrDevice() const;
    void SetCurrDevice(CDevice* device);

    sp<CDevice> CreateDevice(EDeviceCreateHints hints);

private:
    CInstance() = default;
    CInstance(const CInstance&) = delete;
    CInstance(CInstance&&) = delete;
    CInstance& operator=(const CInstance&) = delete;
    CInstance& operator=(CInstance&&) = delete;
};

} /* namespace Nome::RHI */
