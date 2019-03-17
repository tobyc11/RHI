#pragma once
#include "DeviceD3D11.h"
#include "DrawTemplate.h"
#include <VectorPool.h>

namespace RHI
{

/*
IASetInputLayout <= DrawTemplate
IASetPrimitiveTopology <= DrawTemplate
IASetVertexBuffers <= DrawTemplate
IASetIndexBuffer <= DrawTemplate

VS - Shader, CBuffers, Resources <= DrawTemplate

RSSetScissorRects <= Both (only RenderPass for now)
RSSetViewports <= Both (only RenderPass for now)
RSSetState <= DrawTemplate

PS - Shader, CBuffers, Resources <= DrawTemplate

OMSetDepthStencilState <= DrawTemplate
OMSetBlendState <= DrawTemplate
OMSetRenderTargets <= RenderPass
*/

class CDrawCallCacheEntry
{
public:
    bool bIsValid = false;

    D3D11_PRIMITIVE_TOPOLOGY Topology;
    ComPtr<ID3D11InputLayout> InputLayout;
    std::vector<ComPtr<ID3D11Buffer>> VertexBuffers;
    std::vector<uint32_t> Strides;
    std::vector<uint32_t> Offsets;
    ComPtr<ID3D11Buffer> IndexBuffer;
    DXGI_FORMAT IndexFormat;

    uint32_t ElementCount;
    uint32_t InstanceCount;
    uint32_t VertexOffset;
    uint32_t IndexOffset;
    uint32_t InstanceOffset;

    ComPtr<ID3D11RasterizerState> RastState;
    ComPtr<ID3D11DepthStencilState> DepthStencilState;
    ComPtr<ID3D11BlendState> BlendState;

    CShaderD3D11* VertexShader;
    CShaderD3D11* PixelShader;

    //Just keep a copy of the pipeline arguments for now, until we come up with some smarter optimization scheme
    CPipelineArguments PipelineArgs;
};

class CCommandListD3D11
{
public:
    CCommandListD3D11(CDeviceD3D11* parent);

    using CDrawCallCacheEntryRef = tc::TVectorPool<CDrawCallCacheEntry>::VectorPoolReference;
    CDrawCallCacheEntryRef CacheDrawCall(const CDrawTemplate& drawTemplate);
    void RemoveCachedDrawCall(CDrawCallCacheEntryRef cachedDraw);

    void BeginRecording();
    void FinishRecording();
    void Draw(CDrawCallCacheEntryRef cachedDraw);

    ID3D11CommandList* GetD3DCommandList() const { return CmdList.Get(); }

private:
    CDeviceD3D11* Parent;
    ComPtr<ID3D11DeviceContext> Context;
    ComPtr<ID3D11CommandList> CmdList;
    tc::TVectorPool<CDrawCallCacheEntry> DrawCallCache;
};

} /* namespace RHI */
