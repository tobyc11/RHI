#pragma once
#include "DeviceD3D11.h"
#include "DrawTemplate.h"
#include "PipelineCacheD3D11.h"
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

class CCommandListD3D11
{
public:
    CCommandListD3D11(CDeviceD3D11* parent);

    void BeginRecording();
    void FinishRecording();
    void Draw(CPipelineStates states, const CDrawTemplate& draw);
    void SetRenderTargets(const std::vector<CImageView*> color, const CImageView* depthStencil);

    ID3D11CommandList* GetD3DCommandList() const { return CmdList.Get(); }
    void ClearD3DCommandList() { CmdList.Reset(); }

    void SetDefaultViewport(const D3D11_VIEWPORT& vp) { DefaultViewport = vp; }

private:
    CDeviceD3D11* Parent;
    ComPtr<ID3D11DeviceContext> Context;
    ComPtr<ID3D11CommandList> CmdList;
    D3D11_VIEWPORT DefaultViewport;
};

} /* namespace RHI */
