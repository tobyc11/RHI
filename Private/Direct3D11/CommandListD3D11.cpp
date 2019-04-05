#include "CommandListD3D11.h"
#include "ConstantConverter.h"
#include "ShaderD3D11.h"
#include "BufferD3D11.h"
#include "StateCacheD3D11.h"
#include "ImageD3D11.h"
#include "RHIException.h"
#include <Hash.h>
#include <algorithm>
#include <unordered_map>

namespace RHI
{

CCommandListD3D11::CCommandListD3D11(CDeviceD3D11* parent) : Parent(parent)
{
    HRESULT hr = Parent->D3dDevice->CreateDeferredContext(0, Context.GetAddressOf());
    if (!SUCCEEDED(hr))
        throw CRHIRuntimeError("Could not create command list d3d11");
}

void CCommandListD3D11::BeginRecording()
{
}

void CCommandListD3D11::FinishRecording()
{
    CmdList.Reset();
    Context->FinishCommandList(false, CmdList.GetAddressOf());
    Context->ClearState();
}

void CCommandListD3D11::Draw(CPipelineStates states, const CDrawTemplate& draw)
{
    CPipelineStatesD3D11* statesImpl = static_cast<CPipelineStatesD3D11*>(states);
    Context->IASetInputLayout(statesImpl->InputLayout.Get());
    Context->IASetPrimitiveTopology(Convert(statesImpl->PrimitiveTopology));
    Context->RSSetState(statesImpl->RasterizerState.Get());
    Context->OMSetDepthStencilState(statesImpl->DepthStencilState.Get(), 0); //TODO: stencil ref
    Context->OMSetBlendState(statesImpl->BlendState.Get(), nullptr, 0xffffffff); //TODO: blend factor
    if (statesImpl->VertexShader)
        Context->VSSetShader(statesImpl->VertexShader->GetVS(), nullptr, 0);
    if (statesImpl->PixelShader)
        Context->PSSetShader(statesImpl->PixelShader->GetPS(), nullptr, 0);

    //Context->RSSetViewports(1, &DefaultViewport); //TODO: set this only when draw does not

    for (const auto& accessor : draw.GetVertexInputs().BoundAccessors)
    {
        ID3D11Buffer* buffer = static_cast<CBufferD3D11*>(accessor.Buffer.Get())->GetD3D11Buffer();
        uint32_t stride = statesImpl->BindingToStride[accessor.Binding];
        uint32_t offset = accessor.Offset;
        Context->IASetVertexBuffers(accessor.Binding, 1, &buffer, &stride, &offset);
    }
    CBuffer::Ref ib = draw.GetIndexBuffer();
    
    //Update buffers
    for (const auto& req : draw.GetBufferUpdateReqs())
    {
        ID3D11Buffer* buffer = static_cast<CBufferD3D11*>(req.Buffer.Get())->GetD3D11Buffer();
        D3D11_MAPPED_SUBRESOURCE mapped;
        Context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, req.Data, req.Size);
        Context->Unmap(buffer, 0);
    }

    if (statesImpl->VertexShader)
        statesImpl->VertexShader->GetParamMappings().BindArguments<CVSRedir>(draw.GetPipelineArguments(), Context.Get());
    if (statesImpl->PixelShader)
        statesImpl->PixelShader->GetParamMappings().BindArguments<CPSRedir>(draw.GetPipelineArguments(), Context.Get());

    if (ib)
    {
        ID3D11Buffer* ibD3d = static_cast<CBufferD3D11*>(ib.Get())->GetD3D11Buffer();
        Context->IASetIndexBuffer(ibD3d, Convert(draw.GetIndexFormat()), draw.GetIndexBufferOffset());

        if (draw.InstanceCount > 0)
            Context->DrawIndexedInstanced(draw.ElementCount, draw.InstanceCount, draw.IndexOffset, draw.VertexOffset, draw.InstanceOffset);
        else
            Context->DrawIndexed(draw.ElementCount, draw.IndexOffset, draw.VertexOffset);
    }
    else
    {
        if (draw.InstanceCount > 0)
            Context->DrawInstanced(draw.ElementCount, draw.InstanceCount, draw.VertexOffset, draw.InstanceOffset);
        else
            Context->Draw(draw.ElementCount, draw.VertexOffset);
    }
}

void CCommandListD3D11::SetRenderTargets(const std::vector<CImageView*> color, const CImageView* depthStencil)
{
    ComPtr<ID3D11DepthStencilView> dsv;
    if (depthStencil)
        dsv = static_cast<const CImageViewD3D11*>(depthStencil)->GetDepthStencilView();

    std::vector<ID3D11RenderTargetView*> rtvs;
    for (const auto* p : color)
        rtvs.push_back(static_cast<const CImageViewD3D11*>(p)->GetRenderTargetView().Get());

    Context->OMSetRenderTargets((UINT)rtvs.size(), rtvs.data(), dsv.Get());
    Context->RSSetViewports(1, &DefaultViewport);
}

} /* namespace RHI */
