#pragma once
#include "CommandListMetal.h"
#include "CopyContext.h"
#include "ComputeContext.h"
#include "RenderContext.h"

namespace RHI
{

class CCommandContextMetal : public ICopyContext,
                             public IComputeContext,
                             public IRenderContext,
                             public std::enable_shared_from_this<CCommandContextMetal>
{
public:
    typedef std::shared_ptr<CCommandContextMetal> Ref;

    static Ref CreateCopyContext(CCommandListMetal& cmdList);
    static Ref CreateComputeContext(CCommandListMetal& cmdList);
    static Ref CreateRenderContext(id encoder, CDeviceMetal& device);

    ~CCommandContextMetal() override;

    // ICopyContext
    void ClearImage(CImage& image, const CClearValue& clearValue,
                    const CImageSubresourceRange& range) override;
    void CopyBuffer(CBuffer& src, CBuffer& dst,
                    const std::vector<CBufferCopy>& regions) override;
    void CopyImage(CImage& src, CImage& dst, const std::vector<CImageCopy>& regions) override;
    void CopyBufferToImage(CBuffer& src, CImage& dst,
                           const std::vector<CBufferImageCopy>& regions) override;
    void CopyImageToBuffer(CImage& src, CBuffer& dst,
                           const std::vector<CBufferImageCopy>& regions) override;
    void BlitImage(CImage& src, CImage& dst, const std::vector<CImageBlit>& regions,
                   EFilter filter) override;
    void ResolveImage(CImage& src, CImage& dst,
                      const std::vector<CImageResolve>& regions) override;

    // IComputeContext
    void BindComputePipeline(CPipeline& pipeline) override;
    void BindComputeDescriptorSet(uint32_t set, CDescriptorSet& descriptorSet) override;
    void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
    void DispatchIndirect(CBuffer& buffer, size_t offset) override;

    // IRenderContext
    void BindRenderPipeline(CPipeline& pipeline) override;
    void SetViewport(const CViewportDesc& viewportDesc) override;
    void SetScissor(const CRect2D& scissor) override;
    void SetBlendConstants(const std::array<float, 4>& blendConstants) override;
    void SetStencilReference(uint32_t reference) override;
    void BindRenderDescriptorSet(uint32_t set, CDescriptorSet& descriptorSet) override;
    void BindIndexBuffer(CBuffer& buffer, size_t offset, EFormat format) override;
    void BindVertexBuffer(uint32_t binding, CBuffer& buffer, size_t offset) override;
    void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
              uint32_t firstInstance) override;
    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                     int32_t vertexOffset, uint32_t firstInstance) override;
    void DrawIndirect(CBuffer& buffer, size_t offset, uint32_t drawCount,
                      uint32_t stride) override;
    void DrawIndexedIndirect(CBuffer& buffer, size_t offset, uint32_t drawCount,
                             uint32_t stride) override;

    void FinishRecording() override;

    id GetRenderEncoder() const { return RenderEncoder; }
    id GetComputeEncoder() const { return ComputeEncoder; }
    id GetBlitEncoder() const { return BlitEncoder; }

private:
    enum class EContextMode
    {
        Copy,
        Compute,
        Render
    };

    CCommandContextMetal(EContextMode mode, CDeviceMetal& device);

    EContextMode Mode;
    CDeviceMetal& Device;

    id BlitEncoder = nil;
    id ComputeEncoder = nil;
    id RenderEncoder = nil;

    class CPipelineMetal* BoundRenderPipeline = nullptr;
    class CPipelineMetal* BoundComputePipeline = nullptr;
    id BoundIndexBuffer = nil;
    size_t BoundIndexOffset = 0;
    MTLIndexType BoundIndexType = MTLIndexTypeUInt32;
};

class CRenderPassContextMetal : public IParallelRenderContext
{
public:
    typedef std::shared_ptr<CRenderPassContextMetal> Ref;

    CRenderPassContextMetal(CCommandListMetal& cmdList, CRenderPass::Ref renderPass,
                            const std::vector<CClearValue>& clearValues);
    ~CRenderPassContextMetal() override;

    IRenderContext::Ref CreateRenderContext(uint32_t subpass) override;
    void FinishRecording() override;

private:
    CCommandListMetal& CmdList;
    CRenderPass::Ref RenderPass;
    std::vector<CClearValue> ClearValues;
    id ParallelEncoder = nil;
    uint32_t CurrentSubpass = 0;
};

} /* namespace RHI */
