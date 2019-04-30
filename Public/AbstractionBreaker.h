#pragma once
#include "RHIInstance.h"
#ifdef RHI_IMPL_DIRECT3D11
#include <d3d11_4.h>
#include <wrl/client.h>
#elif defined(RHI_IMPL_VULKAN)
#include <vulkan/vulkan.h>
#endif

namespace RHI
{

#ifdef RHI_IMPL_DIRECT3D11
using Microsoft::WRL::ComPtr;

struct CNativeDevice
{
    ComPtr<ID3D11Device> D3dDevice;
    ComPtr<ID3D11DeviceContext> ImmediateContext;
};

CNativeDevice GetNativeDevice(CDevice* device);
#elif defined(RHI_IMPL_VULKAN)
struct CNativeDevice
{
    VkInstance Instance;
    VkDevice Device;
};

CNativeDevice GetNativeDevice(CDevice::Ref device);
#endif

} /* namespace RHI */
