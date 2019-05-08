#pragma once
#include "CommandListVk.h"
#include "ComputeContext.h"
#include "CopyContext.h"
#include "DescriptorSet.h"
#include "RenderContext.h"
#include <SpinLock.h>

namespace RHI
{

struct CSubpassInfo
{
    std::unique_ptr<CCommandBufferVk> SecondaryBuffer;
    CAccessTracker AccessTracker;
};

class CRenderPassContextVk : public std::enable_shared_from_this<CRenderPassContextVk>,
                             public IParallelRenderContext
{
public:
    typedef std::shared_ptr<CRenderPassContextVk> Ref;

    CRenderPassContextVk(CCommandListVk::Ref cmdList, CRenderPass::Ref renderPass,
                         std::vector<CClearValue> clearValues);
    ~CRenderPassContextVk() override;

    CCommandListVk::Ref GetCmdList() const { return CmdList; }
    CRenderPass::Ref GetRenderPass() const { return RenderPass; }
    CSubpassInfo& GetSubpassInfo(uint32_t subpass, uint32_t index)
    {
        return SubpassInfos[subpass][index];
    }
    uint32_t MakeSubpassInfo(uint32_t subpass);

    IRenderContext::Ref CreateRenderContext(uint32_t subpass) override;
    void FinishRecording() override;

private:
    // The target we are recording into
    CCommandListVk::Ref CmdList;
    CRenderPass::Ref RenderPass;
    std::vector<CClearValue> ClearValues;

    // Holds info for render contexts to write to. Cleared when FinishRecording
    tc::FSpinLock SpinLock;
    std::vector<std::vector<CSubpassInfo>> SubpassInfos;
};

class CCommandContextVk : public ICopyContext, public IComputeContext, public IRenderContext
{
    static void Convert(VkOffset2D& dst, const COffset2D& src);
    static void Convert(VkExtent2D& dst, const CExtent2D& src);
    static void Convert(VkOffset3D& dst, const COffset3D& src);
    static void Convert(VkExtent3D& dst, const CExtent3D& src);
    static void Convert(VkImageSubresourceLayers& dst, const CImageSubresourceLayers& src);
    static void Convert(VkImageCopy& dst, const CImageCopy& src);
    static void Convert(VkImageResolve& dst, const CImageResolve& src);
    static void Convert(VkBufferImageCopy& dst, const CBufferImageCopy& src);
    static void Convert(VkImageBlit& dst, const CImageBlit& src);
    static void Convert(VkViewport& dst, const CViewportDesc& src);
    static void Convert(VkRect2D& dst, const CRect2D& src);

public:
    typedef std::shared_ptr<CCommandContextVk> Ref;

    explicit CCommandContextVk(const CCommandListVk::Ref& cmdList);
    explicit CCommandContextVk(const CRenderPassContextVk::Ref& renderPassContext,
                               uint32_t subpass);
    ~CCommandContextVk() override;

    void TransitionImage(CImage& image, EResourceState newState);
    VkCommandBuffer GetCmdBuffer() { return CmdBuffer(); }

    // Copy commands
    void CopyBuffer(CBuffer& src, CBuffer& dst, const std::vector<CBufferCopy>& regions) override;
    void CopyImage(CImage& src, CImage& dst, const std::vector<CImageCopy>& regions) override;
    void CopyBufferToImage(CBuffer& src, CImage& dst,
                           const std::vector<CBufferImageCopy>& regions) override;
    void CopyImageToBuffer(CImage& src, CBuffer& dst,
                           const std::vector<CBufferImageCopy>& regions) override;
    void BlitImage(CImage& src, CImage& dst, const std::vector<CImageBlit>& regions,
                   EFilter filter) override;
    void ResolveImage(CImage& src, CImage& dst, const std::vector<CImageResolve>& regions) override;

    // Compute commands
    void BindComputePipeline(CPipeline& pipeline) override;
    void BindComputeDescriptorSet(uint32_t set, CDescriptorSet& descriptorSet) override;
    void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
    void DispatchIndirect(CBuffer& buffer, size_t offset) override;

    // Render commands
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
    void DrawIndirect(CBuffer& buffer, size_t offset, uint32_t drawCount, uint32_t stride) override;
    void DrawIndexedIndirect(CBuffer& buffer, size_t offset, uint32_t drawCount,
                             uint32_t stride) override;

    // Finish this context and save the commands into the command list
    void FinishRecording() override;

protected:
    CAccessTracker& AccessTracker();
    VkCommandBuffer CmdBuffer();

private:
    // The target we are recording into
    CCommandListVk::Ref CmdList;

    // The target when we are a render pass
    CRenderPassContextVk::Ref RenderPassContext;
    uint32_t SubpassIndex;
    uint32_t CmdBufferIndex;

    // Temporary states
    CPipelineVk* CurrPipeline = nullptr;
};

}
