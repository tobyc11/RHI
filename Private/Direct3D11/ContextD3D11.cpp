#include "ContextD3D11.h"
#include "BufferD3D11.h"
#include "ConstantConverter.h"
#include "DeviceD3D11.h"
#include "ImageD3D11.h"
#include "SamplerD3D11.h"

namespace RHI
{

void CContextD3D11::CopyBuffer(CBuffer* src, CBuffer* dst, const std::vector<CBufferCopy>& regions)
{
    throw "unimplemented";
}

void CContextD3D11::CopyImage(CImage* src, CImage* dst, const std::vector<CImageCopy>& regions)
{
    throw "unimplemented";
}

void CContextD3D11::CopyBufferToImage(CBuffer* src, CImage* dst,
                                      const std::vector<CBufferImageCopy>& regions)
{
    throw "unimplemented";
}

void CContextD3D11::CopyImageToBuffer(CImage* src, CBuffer* dst,
                                      const std::vector<CBufferImageCopy>& regions)
{
    throw "unimplemented";
}

void CContextD3D11::BlitImage(CImage* src, CImage* dst, const std::vector<CImageBlit>& regions,
                              EFilter filter)
{
    throw "unimplemented";
}

void CContextD3D11::ResolveImage(CImage* src, CImage* dst,
                                 const std::vector<CImageResolve>& regions)
{
    throw "unimplemented";
}

void CContextD3D11::ExecuteCommandList(CCommandList* commandList) { throw "unimplemented"; }

CCommandList::Ref CContextD3D11::FinishCommandList() { throw "unimplemented"; }

void CContextD3D11::Flush(bool wait)
{
    if (wait)
        Imm()->Flush();
}

void CContextD3D11::BeginRenderPass(CRenderPass::Ref renderPass,
                                    const std::vector<CClearValue>& clearValues)
{
    CurrPass = renderPass;
    CurrSubpass = 0;
    std::static_pointer_cast<CRenderPassD3D11>(renderPass)->Bind(Imm(), CurrSubpass++, clearValues);
}

void CContextD3D11::NextSubpass()
{
    std::static_pointer_cast<CRenderPassD3D11>(CurrPass)->Bind(Imm(), CurrSubpass++, {});
    // clear states to match vulkan behavior
    Imm()->ClearState();
    CurrPass.reset();
    CurrPipeline.reset();
    VSBoundLayouts.clear();
    PSBoundLayouts.clear();
    BoundResources.clear();
}

void CContextD3D11::EndRenderPass()
{
    Imm()->ClearState();
    CurrPass.reset();
    CurrPipeline.reset();
    VSBoundLayouts.clear();
    PSBoundLayouts.clear();
    BoundResources.clear();
}

void CContextD3D11::BindPipeline(CPipeline::Ref pipeline)
{
    CurrPipeline = std::static_pointer_cast<CPipelineD3D11>(pipeline);
    Imm()->IASetInputLayout(CurrPipeline->InputLayout.Get());
    Imm()->IASetPrimitiveTopology(Convert(CurrPipeline->PrimitiveTopology));
    Imm()->RSSetState(CurrPipeline->RasterizerState.Get());
    Imm()->OMSetDepthStencilState(CurrPipeline->DepthStencilState.Get(), 0);
    Imm()->OMSetBlendState(CurrPipeline->BlendState.Get(), nullptr, 0xffffffff);
    Imm()->VSSetShader(CurrPipeline->VS.Get(), nullptr, 0);
    Imm()->PSSetShader(CurrPipeline->PS.Get(), nullptr, 0);
}

void CContextD3D11::BindBuffer(CBuffer::Ref buffer, size_t offset, size_t range, uint32_t set,
                               uint32_t binding, uint32_t index)
{
    CBoundResource bound {};
    bound.Buffer = std::static_pointer_cast<CBufferD3D11>(buffer)->GetD3D11Buffer();
    bound.Offset = offset;
    bound.Range = range;
    BoundResources[MakeTriple(set, binding, index)] = bound;
}

void CContextD3D11::BindBufferView(CBufferView::Ref bufferView, uint32_t set, uint32_t binding,
                                   uint32_t index)
{
    throw "unimplemented";
}

void CContextD3D11::BindImageView(CImageView::Ref imageView, uint32_t set, uint32_t binding,
                                  uint32_t index)
{
    CBoundResource bound {};
    bound.ImageView =
        std::static_pointer_cast<CImageViewD3D11>(imageView)->GetShaderResourceView().Get();
    BoundResources[MakeTriple(set, binding, index)] = bound;
}

void CContextD3D11::BindSampler(CSampler::Ref sampler, uint32_t set, uint32_t binding,
                                uint32_t index)
{
    CBoundResource bound {};
    bound.Sampler = std::static_pointer_cast<CSamplerD3D11>(sampler)->GetSamplerState();
    BoundResources[MakeTriple(set, binding, index)] = bound;
}

void CContextD3D11::BindIndexBuffer(CBuffer::Ref buffer, size_t offset, EFormat format)
{
    auto impl = std::static_pointer_cast<CBufferD3D11>(buffer);
    Imm()->IASetIndexBuffer(impl->GetD3D11Buffer(), Convert(format), static_cast<UINT>(offset));
}

void CContextD3D11::BindVertexBuffer(uint32_t binding, CBuffer::Ref buffer, size_t offset)
{
    auto impl = std::static_pointer_cast<CBufferD3D11>(buffer);
    ID3D11Buffer* buf = impl->GetD3D11Buffer();
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

ID3D11DeviceContext* CContextD3D11::Imm() { return Parent.ImmediateContext.Get(); }

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
