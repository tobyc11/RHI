#pragma once
#include "Device.h"
#include "D3D11Platform.h"

namespace Nome::RHI
{

class CDeviceD3D11 : public CDeviceBase<CDeviceD3D11>
{
public:
    CDeviceD3D11(EDeviceCreateHints hints);

private:
    D3D_DRIVER_TYPE DriverType;
    D3D_FEATURE_LEVEL FeatureLevel;
    ComPtr<ID3D11Device> Device;
    ComPtr<ID3D11DeviceContext> ImmediateContext;
    ComPtr<IDXGIFactory1> DxgiFactory;
};

} /* namespace Nome::RHI */
