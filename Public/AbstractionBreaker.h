#pragma once
#include "RHIInstance.h"
#ifdef RHI_IMPL_DIRECT3D11
#include <d3d11_4.h>
#include <wrl/client.h>
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

//This feature lets you to break the abstraction barrier and access the lower level features directly
class CNativeRenderPass : public CRenderPass
{
public:
    CNativeRenderPass(CRenderGraph& renderGraph)
        : CRenderPass(renderGraph)
    {
        CNativeDevice native = GetNativeDevice(CInstance::Get().GetCurrDevice());
        D3dDevice = native.D3dDevice;
        ImmediateContext = native.ImmediateContext;
    }

    void Submit() const override = 0;

protected:
    void BindRenderTargets() const;

    ComPtr<ID3D11Device> D3dDevice;
    ComPtr<ID3D11DeviceContext> ImmediateContext;
};
#endif

} /* namespace RHI */
