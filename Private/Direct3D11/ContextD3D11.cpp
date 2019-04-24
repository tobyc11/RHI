#include "ContextD3D11.h"
#include "BufferD3D11.h"
#include "ConstantConverter.h"
#include "DeviceD3D11.h"
#include "ImageD3D11.h"
#include "SamplerD3D11.h"

namespace RHI
{

CContextD3D11::CContextD3D11(CDeviceD3D11& p, bool isDeferred)
    : Parent(p)
{
    if (isDeferred)
    {
        Parent.D3dDevice->CreateDeferredContext(0, DeferredContext.GetAddressOf());
    }
}

void CContextD3D11::CopyBuffer(CBuffer& src, CBuffer& dst, const std::vector<CBufferCopy>& regions)
{
    auto& srcImpl = static_cast<CBufferD3D11&>(src);
    auto& dstImpl = static_cast<CBufferD3D11&>(dst);
    for (const auto& region : regions)
    {
        D3D11_BOX srcBox;
        srcBox.left = static_cast<UINT>(region.SrcOffset);
        srcBox.right = static_cast<UINT>(region.SrcOffset) + static_cast<UINT>(region.Size);
        srcBox.top = 0;
        srcBox.bottom = 1;
        srcBox.front = 0;
        srcBox.back = 1;
        Imm()->CopySubresourceRegion(dstImpl.GetD3D11Buffer(), 0,
                                     static_cast<UINT>(region.DstOffset), 0, 0,
                                     srcImpl.GetD3D11Buffer(), 0, &srcBox);
    }
}

void CContextD3D11::CopyImage(CImage& src, CImage& dst, const std::vector<CImageCopy>& regions)
{
    throw "unimplemented";
}

void CContextD3D11::CopyBufferToImage(CBuffer& src, CImage& dst,
                                      const std::vector<CBufferImageCopy>& regions)
{
    throw "unimplemented";
}

void CContextD3D11::CopyImageToBuffer(CImage& src, CBuffer& dst,
                                      const std::vector<CBufferImageCopy>& regions)
{
    throw "unimplemented";
}

void CContextD3D11::BlitImage(CImage& src, CImage& dst, const std::vector<CImageBlit>& regions,
                              EFilter filter)
{
    throw "unimplemented";
}

void CContextD3D11::ResolveImage(CImage& src, CImage& dst,
                                 const std::vector<CImageResolve>& regions)
{
    throw "unimplemented";
}

void CContextD3D11::ExecuteCommandList(CCommandList& commandList)
{
    auto impl = static_cast<CCommandListD3D11&>(commandList);
    Imm()->ExecuteCommandList(impl.CommandList.Get(), FALSE);
}

CCommandList::Ref CContextD3D11::FinishCommandList()
{
    auto cmdList = std::make_shared<CCommandListD3D11>();
    Imm()->FinishCommandList(FALSE, cmdList->CommandList.GetAddressOf());
    return std::move(cmdList);
}

void CContextD3D11::Flush(bool wait)
{
    if (wait)
        Imm()->Flush();
}

void CContextD3D11::BeginRenderPass(CRenderPass& renderPass,
                                    const std::vector<CClearValue>& clearValues)
{
    CurrPass = &renderPass;
    CurrSubpass = 0;
    static_cast<CRenderPassD3D11*>(CurrPass)->Bind(Imm(), CurrSubpass++, clearValues);
}

void CContextD3D11::NextSubpass()
{
    static_cast<CRenderPassD3D11*>(CurrPass)->Bind(Imm(), CurrSubpass++, {});
}

void CContextD3D11::EndRenderPass()
{
    CurrPass = nullptr;
    // Clearing all bindings isn't exactly the expected behavior, but I don't know whereelse to put it
    CurrPipeline = nullptr;
    VSBoundLayouts.clear();
    PSBoundLayouts.clear();
    BoundResources.clear();
}

void CContextD3D11::BindPipeline(CPipeline& pipeline)
{
    CurrPipeline = static_cast<CPipelineD3D11*>(&pipeline);
    Imm()->IASetInputLayout(CurrPipeline->InputLayout.Get());
    Imm()->IASetPrimitiveTopology(Convert(CurrPipeline->PrimitiveTopology));
    Imm()->RSSetState(CurrPipeline->RasterizerState.Get());
    Imm()->OMSetDepthStencilState(CurrPipeline->DepthStencilState.Get(), 0);
    Imm()->OMSetBlendState(CurrPipeline->BlendState.Get(), nullptr, 0xffffffff);
    Imm()->VSSetShader(CurrPipeline->VS.Get(), nullptr, 0);
    Imm()->PSSetShader(CurrPipeline->PS.Get(), nullptr, 0);
}

void CContextD3D11::BindBuffer(CBuffer& buffer, size_t offset, size_t range, uint32_t set,
                               uint32_t binding, uint32_t index)
{
    auto location = MakeTriple(set, binding, index);
    auto iter = BoundResources.find(location);
    if (iter == BoundResources.end())
    {
        CBoundResource bound {};
        bound.Buffer = static_cast<CBufferD3D11&>(buffer).GetD3D11Buffer();
        bound.Offset = offset;
        bound.Range = range;
        BoundResources[MakeTriple(set, binding, index)] = bound;
    }
    else
    {
        iter->second.SetBuffer(static_cast<CBufferD3D11&>(buffer).GetD3D11Buffer(), offset, range);
    }
    assert(offset == 0); // D3D11.0 does not support offset in cbuffer binding
}

void CContextD3D11::BindBufferView(CBufferView& bufferView, uint32_t set, uint32_t binding,
                                   uint32_t index)
{
    throw "unimplemented";
}

void RHI::CContextD3D11::BindConstants(const void* pData, size_t size, uint32_t set,
                                       uint32_t binding, uint32_t index)
{
    auto location = MakeTriple(set, binding, index);
    auto iter = BoundResources.find(location);
    if (iter == BoundResources.end())
    {
        CBoundResource bound {};
        // Create a transient (no really) constant buffer for this slot
        D3D11_BUFFER_DESC bd = {};
        bd.ByteWidth = static_cast<UINT>(size) * 2;
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        Parent.D3dDevice->CreateBuffer(&bd, nullptr, bound.TransientCBuffer.GetAddressOf());
        bound.TransientCBufferSize = bd.ByteWidth;

        // Set buffer and record the binding
        bound.Buffer = bound.TransientCBuffer.Get();
        D3D11_MAPPED_SUBRESOURCE mapped;
        Imm()->Map(bound.TransientCBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, pData, size);
        Imm()->Unmap(bound.TransientCBuffer.Get(), 0);
        BoundResources.emplace(location, std::move(bound));
    }
    else
    {
        if (iter->second.TransientCBufferSize < size)
        {
            // Recreate the buffer since it's too small
            iter->second.TransientCBuffer.Reset();
            D3D11_BUFFER_DESC bd = {};
            bd.ByteWidth = static_cast<UINT>(size) * 2;
            bd.Usage = D3D11_USAGE_DYNAMIC;
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            Parent.D3dDevice->CreateBuffer(&bd, nullptr,
                                           iter->second.TransientCBuffer.GetAddressOf());
            iter->second.TransientCBufferSize = bd.ByteWidth;
        }
        D3D11_MAPPED_SUBRESOURCE mapped;
        Imm()->Map(iter->second.TransientCBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, pData, size);
        Imm()->Unmap(iter->second.TransientCBuffer.Get(), 0);
        iter->second.SetBuffer(iter->second.TransientCBuffer.Get(), 0, 0);
    }
}

void CContextD3D11::BindImageView(CImageView& imageView, uint32_t set, uint32_t binding,
                                  uint32_t index)
{
    auto location = MakeTriple(set, binding, index);
    auto iter = BoundResources.find(location);
    if (iter == BoundResources.end())
    {
        CBoundResource bound {};
        bound.ImageView = static_cast<CImageViewD3D11&>(imageView).GetShaderResourceView().Get();
        BoundResources[MakeTriple(set, binding, index)] = bound;
    }
    else
    {
        iter->second.SetImageView(
            static_cast<CImageViewD3D11&>(imageView).GetShaderResourceView().Get());
    }
}

void CContextD3D11::BindSampler(CSampler& sampler, uint32_t set, uint32_t binding, uint32_t index)
{
    auto location = MakeTriple(set, binding, index);
    auto iter = BoundResources.find(location);
    if (iter == BoundResources.end())
    {
        CBoundResource bound {};
        bound.SetSampler(static_cast<CSamplerD3D11&>(sampler).GetSamplerState());
        BoundResources[MakeTriple(set, binding, index)] = bound;
    }
    else
    {
        iter->second.SetSampler(static_cast<CSamplerD3D11&>(sampler).GetSamplerState());
    }
}

void CContextD3D11::BindIndexBuffer(CBuffer& buffer, size_t offset, EFormat format)
{
    auto& impl = static_cast<CBufferD3D11&>(buffer);
    Imm()->IASetIndexBuffer(impl.GetD3D11Buffer(), Convert(format), static_cast<UINT>(offset));
}

void CContextD3D11::BindVertexBuffer(uint32_t binding, CBuffer& buffer, size_t offset)
{
    auto& impl = static_cast<CBufferD3D11&>(buffer);
    ID3D11Buffer* buf = impl.GetD3D11Buffer();
    uint32_t stride = CurrPipeline->BindingToStride[binding];
    UINT off = static_cast<UINT>(offset);
    Imm()->IASetVertexBuffers(binding, 1, &buf, &stride, &off);
}

void CContextD3D11::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                         uint32_t firstInstance)
{
    ResolveResourceBindings();
    if (instanceCount > 1)
        Imm()->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
    else
        Imm()->Draw(vertexCount, firstInstance);
}

void CContextD3D11::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                int32_t vertexOffset, uint32_t firstInstance)
{
    ResolveResourceBindings();
    if (instanceCount > 1)
        Imm()->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset,
                                    firstInstance);
    else
        Imm()->DrawIndexed(indexCount, firstIndex, vertexOffset);
}

ID3D11DeviceContext* CContextD3D11::Imm()
{
    if (DeferredContext)
        return DeferredContext.Get();
    return Parent.ImmediateContext.Get();
}

void RHI::CContextD3D11::ResolveResourceBindings()
{
    // For vertex shader
    for (const auto& setAndLayout : CurrPipeline->VSDescriptorSetLayouts)
    {
        uint32_t set = setAndLayout.first;
        bool doRebind = false;

        // No layout is bound at the current set
        const CDescriptorSetRemap* layout = nullptr;
        auto boundSet = VSBoundLayouts.find(set);
        if (boundSet == VSBoundLayouts.end())
        {
            VSBoundLayouts[set] = layout = setAndLayout.second;
            doRebind = true;
        }
        else if (boundSet->second != setAndLayout.second)
        {
            boundSet->second = layout = setAndLayout.second;
            doRebind = true;
        }

        if (doRebind)
        {
            auto rsrcBegin = BoundResources.lower_bound(MakeTriple(set, 0, 0));
            auto rsrcEnd = BoundResources.lower_bound(MakeTriple(set + 1, 0, 0));
            for (auto iter = rsrcBegin; iter != rsrcEnd; ++iter)
            {
                uint32_t binding = iter->first.second.first;
                if (iter->second.Buffer)
                {
                    uint32_t slot = layout->ConstantBuffers.at(binding);
                    Imm()->VSSetConstantBuffers(slot, 1, &iter->second.Buffer);
                }
                else if (iter->second.ImageView)
                {
                    uint32_t slot = layout->Textures.at(binding);
                    Imm()->VSSetShaderResources(slot, 1, &iter->second.ImageView);
                }
                else if (iter->second.Sampler)
                {
                    uint32_t slot = layout->Samplers.at(binding);
                    Imm()->VSSetSamplers(slot, 1, &iter->second.Sampler);
                }
            }
        }
    }
    // Same code, except for pixel shader
    for (const auto& setAndLayout : CurrPipeline->PSDescriptorSetLayouts)
    {
        uint32_t set = setAndLayout.first;
        bool doRebind = false;

        // No layout is bound at the current set
        const CDescriptorSetRemap* layout = nullptr;
        auto boundSet = PSBoundLayouts.find(set);
        if (boundSet == PSBoundLayouts.end())
        {
            PSBoundLayouts[set] = layout = setAndLayout.second;
            doRebind = true;
        }
        else if (boundSet->second != setAndLayout.second)
        {
            boundSet->second = layout = setAndLayout.second;
            doRebind = true;
        }

        if (doRebind)
        {
            auto rsrcBegin = BoundResources.lower_bound(MakeTriple(set, 0, 0));
            auto rsrcEnd = BoundResources.lower_bound(MakeTriple(set + 1, 0, 0));
            for (auto iter = rsrcBegin; iter != rsrcEnd; ++iter)
            {
                uint32_t binding = iter->first.second.first;
                if (iter->second.Buffer)
                {
                    uint32_t slot = layout->ConstantBuffers.at(binding);
                    Imm()->PSSetConstantBuffers(slot, 1, &iter->second.Buffer);
                }
                else if (iter->second.ImageView)
                {
                    uint32_t slot = layout->Textures.at(binding);
                    Imm()->PSSetShaderResources(slot, 1, &iter->second.ImageView);
                }
                else if (iter->second.Sampler)
                {
                    uint32_t slot = layout->Samplers.at(binding);
                    Imm()->PSSetSamplers(slot, 1, &iter->second.Sampler);
                }
            }
        }
    }
}

}
