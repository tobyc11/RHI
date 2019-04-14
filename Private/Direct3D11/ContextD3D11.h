#pragma once
#include "D3D11Platform.h"
#include "PipelineD3D11.h"
#include "RenderContext.h"
#include "RenderPassD3D11.h"
#include <map>
#include <utility>

namespace RHI
{

struct CBoundResource
{
    ID3D11Buffer* Buffer;
    size_t Offset;
    size_t Range;
    ID3D11ShaderResourceView* ImageView;
    ID3D11SamplerState* Sampler;

    ComPtr<ID3D11Buffer> TransientCBuffer;
    uint32_t TransientCBufferSize;

    void SetBuffer(ID3D11Buffer* buffer, size_t offset, size_t range)
    {
        Buffer = buffer;
        Offset = offset;
        Range = range;
        ImageView = nullptr;
        Sampler = nullptr;
    }

    void SetImageView(ID3D11ShaderResourceView* srv)
    {
        Buffer = nullptr;
        ImageView = srv;
        Sampler = nullptr;
    }

    void SetSampler(ID3D11SamplerState* sampler)
    {
        Buffer = nullptr;
        ImageView = nullptr;
        Sampler = sampler;
    }
};

class CContextD3D11 : public IRenderContext
{
public:
    CContextD3D11(CDeviceD3D11& p)
        : Parent(p)
    {
    }

    // Inherited via ICopyContext
    void CopyBuffer(CBuffer* src, CBuffer* dst, const std::vector<CBufferCopy>& regions) override;
    void CopyImage(CImage* src, CImage* dst, const std::vector<CImageCopy>& regions) override;
    void CopyBufferToImage(CBuffer* src, CImage* dst,
                           const std::vector<CBufferImageCopy>& regions) override;
    void CopyImageToBuffer(CImage* src, CBuffer* dst,
                           const std::vector<CBufferImageCopy>& regions) override;
    void BlitImage(CImage* src, CImage* dst, const std::vector<CImageBlit>& regions,
                   EFilter filter) override;
    void ResolveImage(CImage* src, CImage* dst, const std::vector<CImageResolve>& regions) override;
    void ExecuteCommandList(CCommandList* commandList) override;
    CCommandList::Ref FinishCommandList() override;
    void Flush(bool wait = false) override;

    // Inherited via IRenderContext
    void BeginRenderPass(CRenderPass::Ref renderPass,
                         const std::vector<CClearValue>& clearValues) override;
    void NextSubpass() override;
    void EndRenderPass() override;
    void BindPipeline(CPipeline::Ref pipeline) override;
    void BindBuffer(CBuffer::Ref buffer, size_t offset, size_t range, uint32_t set,
                    uint32_t binding, uint32_t index) override;
    void BindBufferView(CBufferView::Ref bufferView, uint32_t set, uint32_t binding,
                        uint32_t index) override;
    void BindConstants(const void* pData, size_t size, uint32_t set, uint32_t binding,
                       uint32_t index) override;
    void BindImageView(CImageView::Ref imageView, uint32_t set, uint32_t binding,
                       uint32_t index) override;
    void BindSampler(CSampler::Ref sampler, uint32_t set, uint32_t binding,
                     uint32_t index) override;
    void BindIndexBuffer(CBuffer::Ref buffer, size_t offset, EFormat format) override;
    void BindVertexBuffer(uint32_t binding, CBuffer::Ref buffer, size_t offset) override;
    void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
              uint32_t firstInstance) override;
    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                     int32_t vertexOffset, uint32_t firstInstance) override;

private:
    CDeviceD3D11& Parent;
    ID3D11DeviceContext* Imm();
    void ResolveResourceBindings();
    static std::pair<uint32_t, std::pair<uint32_t, uint32_t>>
    MakeTriple(uint32_t set, uint32_t binding, uint32_t index)
    {
        return std::make_pair(set, std::make_pair(binding, index));
    }

    CRenderPass::Ref CurrPass;
    size_t CurrSubpass = 0;
    CPipelineD3D11::Ref CurrPipeline;
    std::unordered_map<uint32_t, const CDescriptorSetRemap*> VSBoundLayouts;
    std::unordered_map<uint32_t, const CDescriptorSetRemap*> PSBoundLayouts;
    std::map<std::pair<uint32_t, std::pair<uint32_t, uint32_t>>, CBoundResource> BoundResources;
};

}