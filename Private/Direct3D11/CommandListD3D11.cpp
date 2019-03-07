#include "CommandListD3D11.h"
#include "ConstantConverter.h"
#include "ShaderD3D11.h"
#include "BufferD3D11.h"
#include "StateCacheD3D11.h"
#include "RHIException.h"
#include <Hash.h>
#include <unordered_map>
#include <mutex>

namespace Nome::RHI
{

CCommandListD3D11::CCommandListD3D11(CDeviceD3D11* parent) : Parent(parent)
{
    HRESULT hr = Parent->D3dDevice->CreateDeferredContext(0, DeferredCtx.GetAddressOf());
    if (!SUCCEEDED(hr))
        throw CRHIRuntimeError("Could not create command list d3d11");
}

CCommandListD3D11::CDrawCallCacheEntryRef CCommandListD3D11::CacheDrawCall(const CDrawTemplate& drawTemplate)
{
    auto result = DrawCallCache.GetNewElement(tc::kAddToBack);

    //static_assert(sizeof(CViewportDesc) == sizeof(D3D11_VIEWPORT), "D3D11 viewport and CViewportDesc incompatible.");
    //DeferredCtx->RSSetViewports((UINT)drawTemplate.GetViewports().size(), (D3D11_VIEWPORT*)drawTemplate.GetViewports().data());
    //static_assert(sizeof(CRectDesc) == sizeof(D3D11_RECT), "D3D11 rect and CRectDecs incompatible.");
    //DeferredCtx->RSSetScissorRects((UINT)drawTemplate.GetScissors().size(), (D3D11_RECT*)drawTemplate.GetScissors().data());

    result->RastState = Parent->StateCache->FindOrCreate(drawTemplate.GetRasterizerDesc());
    result->DepthStencilState = Parent->StateCache->FindOrCreate(drawTemplate.GetDepthStencilDesc());
    result->BlendState = Parent->StateCache->FindOrCreate(drawTemplate.GetBlendDesc());

    new CShaderD3D11(*drawTemplate.GetVertexShader());

    const size_t kNumBuffers = 16;
    ID3D11Buffer* buffers[kNumBuffers] = { 0 };
    uint32_t strides[kNumBuffers] = { 0 };
    uint32_t offsets[kNumBuffers] = { 0 };
    const auto& vertexInputBinding = drawTemplate.GetVertexInputBinding();
    for (const auto& bindingPair : vertexInputBinding.LocationToAccessor)
    {
        auto* bufferImpl = static_cast<CBufferD3D11*>(bindingPair.second.Buffer.get());
        buffers[bindingPair.first] = bufferImpl->GetD3D11Buffer();
        strides[bindingPair.first] = bindingPair.second.Stride;
        offsets[bindingPair.first] = bindingPair.second.Offset;
    }
    DeferredCtx->IASetVertexBuffers(0, kNumBuffers, buffers, strides, offsets);

    return result;
}

} /* namespace Nome::RHI */
