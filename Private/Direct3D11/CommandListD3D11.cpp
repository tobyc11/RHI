#include "CommandListD3D11.h"
#include "ConstantConverter.h"
#include "ShaderD3D11.h"
#include "BufferD3D11.h"
#include "StateCacheD3D11.h"
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

CCommandListD3D11::CDrawCallCacheEntryRef CCommandListD3D11::CacheDrawCall(const CDrawTemplate& drawTemplate)
{
    auto result = DrawCallCache.GetNewElement(tc::kAddToBack);

    static_assert(sizeof(CViewportDesc) == sizeof(D3D11_VIEWPORT), "D3D11 viewport and CViewportDesc incompatible.");
    //Context->RSSetViewports((UINT)drawTemplate.GetViewports().size(), (D3D11_VIEWPORT*)drawTemplate.GetViewports().data());
    static_assert(sizeof(CRectDesc) == sizeof(D3D11_RECT), "D3D11 rect and CRectDecs incompatible.");
    //Context->RSSetScissorRects((UINT)drawTemplate.GetScissors().size(), (D3D11_RECT*)drawTemplate.GetScissors().data());

    result->RastState = Parent->StateCache->FindOrCreate(drawTemplate.GetRasterizerDesc());
    result->DepthStencilState = Parent->StateCache->FindOrCreate(drawTemplate.GetDepthStencilDesc());
    result->BlendState = Parent->StateCache->FindOrCreate(drawTemplate.GetBlendDesc());

    auto vs = drawTemplate.GetVertexShader();
    std::string key = vs->GetShaderCacheKey();
    if (!(result->VertexShader = Parent->ShaderCache->GetShader(key)))
    {
        result->VertexShader = new CShaderD3D11(*vs);
        result->VertexShader->GetVS(); //Triggers CreateVertexShader
        Parent->ShaderCache->PutShader(key, result->VertexShader);
    }

    auto ps = drawTemplate.GetVertexShader();
    key = ps->GetShaderCacheKey();
    if (!(result->PixelShader = Parent->ShaderCache->GetShader(key)))
    {
        result->PixelShader = new CShaderD3D11(*ps);
        result->PixelShader->GetPS(); //Triggers CreatePixelShader
        Parent->ShaderCache->PutShader(key, result->PixelShader);
    }

    std::vector<D3D11_INPUT_ELEMENT_DESC> inputDescs;
    auto& accessors = drawTemplate.GetVertexInputBinding().BoundAccessors;
    std::sort(accessors.begin(), accessors.end(), CVertexShaderInputBinding::CBufferAndStrideComparator());
    sp<CBuffer> groupBuf;
    uint32_t groupStride = static_cast<uint32_t>(-1);
    size_t groupBegin = SIZE_MAX;
    uint32_t groupMinOffset = INT_MAX;
    for (size_t i = 0; i <= accessors.size(); i++) //Intentially loop over one more index
    {
        //Starting a new group?
        if (i == accessors.size() || accessors[i].Stride != groupStride || accessors[i].Buffer != groupBuf)
        {
            if (i != 0)
            {
                //Finalize last group
                uint32_t bufferSlot = static_cast<uint32_t>(result->VertexBuffers.size());
                auto* bufferImpl = static_cast<CBufferD3D11*>(accessors[groupBegin].Buffer.Get());
                result->VertexBuffers.emplace_back(bufferImpl->GetD3D11Buffer());
                result->Strides.emplace_back(groupStride);
                result->Offsets.emplace_back(groupMinOffset);

                for (size_t j = groupBegin; j < i; j++)
                {
                    auto iter = result->VertexShader->GetVSInputSignature().InputDescs.find(accessors[j].Location);
                    if (iter == result->VertexShader->GetVSInputSignature().InputDescs.end())
                        throw CRHIRuntimeError("Vertex input binding names a nonexistent location");

                    D3D11_INPUT_ELEMENT_DESC inputDesc;
                    inputDesc.SemanticName = "ATTRIBUTE"; //TODO: let shader provide a way to map "location" to semantic strings
                    inputDesc.SemanticIndex = accessors[j].Location;
                    inputDesc.Format = Convert(iter->second.Format);
                    inputDesc.InputSlot = bufferSlot;
                    inputDesc.AlignedByteOffset = accessors[j].Offset - groupMinOffset;
                    inputDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                    inputDesc.InstanceDataStepRate = 0;
                    inputDescs.push_back(inputDesc);
                }
            }
            if (i != accessors.size())
            {
                //Initialize next group
                groupBuf = accessors[i].Buffer;
                groupStride = accessors[i].Stride;
                groupBegin = i;
                groupMinOffset = accessors[i].Offset;
            }
        }
        if (i != accessors.size())
        {
            groupMinOffset = min(groupMinOffset, accessors[i].Offset);
        }
    }
    auto vsCode = result->VertexShader->GetCodeBlob();
    Parent->D3dDevice->CreateInputLayout(inputDescs.data(), static_cast<UINT>(inputDescs.size()),
        vsCode->GetBufferPointer(), vsCode->GetBufferSize(), result->InputLayout.GetAddressOf());

    result->Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; //TODO: fixme
    if (drawTemplate.GetIndexBuffer())
    {
        auto* bufferImpl = static_cast<CBufferD3D11*>(drawTemplate.GetIndexBuffer().Get());
        result->IndexBuffer = bufferImpl->GetD3D11Buffer();
        if (drawTemplate.IndexWidth == 4)
            result->IndexFormat = DXGI_FORMAT_R32_UINT;
        else if (drawTemplate.IndexWidth == 2)
            result->IndexFormat = DXGI_FORMAT_R16_UINT;
        else if (drawTemplate.IndexWidth == 1)
            result->IndexFormat = DXGI_FORMAT_R8_UINT;
        else
            throw CRHIRuntimeError("drawTemplate.IndexWidth incorrect");
    }

    result->ElementCount = drawTemplate.ElementCount;
    result->InstanceCount = drawTemplate.InstanceCount;
    result->VertexOffset = drawTemplate.VertexOffset;
    result->IndexOffset = drawTemplate.IndexOffset;
    result->InstanceOffset = drawTemplate.InstanceOffset;

    result->PipelineArgs = drawTemplate.GetPipelineArguments();

    result->bIsValid = true;
    return result;
}

void CCommandListD3D11::RemoveCachedDrawCall(CDrawCallCacheEntryRef cachedDraw)
{
    DrawCallCache.FreeElement(cachedDraw);
}

void CCommandListD3D11::BeginRecording()
{
    Context->ClearState();
}

void CCommandListD3D11::FinishRecording()
{
    Context->FinishCommandList(false, CmdList.GetAddressOf());
}

void CCommandListD3D11::Draw(CDrawCallCacheEntryRef cachedDraw)
{
    Context->IASetPrimitiveTopology(cachedDraw->Topology);
    Context->IASetInputLayout(cachedDraw->InputLayout.Get());
    static_assert(sizeof(ComPtr<ID3D11Buffer>) == sizeof(ID3D11Buffer*), "Smart pointer size too big");
    uint32_t numBuffers = cachedDraw->VertexBuffers.size();
    Context->IASetVertexBuffers(0, numBuffers,
        reinterpret_cast<ID3D11Buffer**>(cachedDraw->VertexBuffers.data()), cachedDraw->Strides.data(), cachedDraw->Offsets.data());
    Context->IASetIndexBuffer(cachedDraw->IndexBuffer.Get(), cachedDraw->IndexFormat, 0); //TODO: specify IB offset

    Context->RSSetState(cachedDraw->RastState.Get());
    Context->OMSetDepthStencilState(cachedDraw->DepthStencilState.Get(), 0); //TODO: stencil ref
    Context->OMSetBlendState(cachedDraw->BlendState.Get(), nullptr, 0xffffffff);

    Context->VSSetShader(cachedDraw->VertexShader->GetVS(), nullptr, 0);
    Context->PSSetShader(cachedDraw->PixelShader->GetPS(), nullptr, 0);

    cachedDraw->VertexShader->GetParamMappings().BindArguments<CVSRedir>(cachedDraw->PipelineArgs, Context.Get());
    cachedDraw->PixelShader->GetParamMappings().BindArguments<CPSRedir>(cachedDraw->PipelineArgs, Context.Get());

    if (cachedDraw->IndexBuffer)
    {
        if (cachedDraw->InstanceCount > 0)
            Context->DrawIndexedInstanced(cachedDraw->ElementCount, cachedDraw->InstanceCount,
                cachedDraw->IndexOffset, cachedDraw->VertexOffset, cachedDraw->InstanceOffset);
        else
            Context->DrawIndexed(cachedDraw->ElementCount, cachedDraw->IndexOffset, cachedDraw->VertexOffset);
    }
    else
    {
        if (cachedDraw->InstanceCount > 0)
            Context->DrawInstanced(cachedDraw->ElementCount, cachedDraw->InstanceCount,
                cachedDraw->VertexOffset, cachedDraw->InstanceOffset);
        else
            Context->Draw(cachedDraw->ElementCount, cachedDraw->VertexOffset);
    }
}

} /* namespace RHI */
