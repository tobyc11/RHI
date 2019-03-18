#pragma once
#include "PipelineCache.h"
#include "DeviceD3D11.h"
#include "ShaderD3D11.h"
#include "D3D11Platform.h"

#include <map>

namespace RHI
{

class CPipelineStatesD3D11
{
public:
    std::map<uint32_t, uint32_t> BindingToStride;
    ComPtr<ID3D11InputLayout> InputLayout;
    EPrimitiveTopology PrimitiveTopology;
    ComPtr<ID3D11RasterizerState> RasterizerState;
    ComPtr<ID3D11DepthStencilState> DepthStencilState;
    ComPtr<ID3D11BlendState> BlendState;
    CShaderD3D11* VertexShader;
    CShaderD3D11* PixelShader;
};

class CPipelineCacheD3D11 : public CPipelineCacheBase<CPipelineCacheD3D11>
{
public:
    explicit CPipelineCacheD3D11(CDeviceD3D11& device) : DeviceImpl(device) {}

    CPipelineStates CreatePipelineStates(const CDrawTemplate& dt);
    void DestroyPipelineStates(CPipelineStates pso);

private:
    CDeviceD3D11& DeviceImpl;
};

} /* namespace RHI */
