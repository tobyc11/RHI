#pragma once
#include "BufferVk.h"
#include "ImageVk.h"
#include "PipelineVk.h"
#include "RenderContext.h"
#include "ResourceBindingsVk.h"
#include "SubmissionTracker.h"
#include "VkCommon.h"
#include <map>
#include <memory>
#include <mutex>
#include <vector>

namespace RHI
{

class CCommandListVk : public CCommandList
{
public:
    typedef std::shared_ptr<CCommandListVk> Ref;

    ~CCommandListVk();

    bool IsRecording() const { return bCurrentlyRecording; }
    void SetRecording(bool r) { bCurrentlyRecording = r; }
    CAccessTracker& GetAccessTracker() { return AccessTracker; }
    void AppendPrimaryCommandBuffer(std::unique_ptr<CCommandBufferVk> buffer)
    {
        CmdBuffers.push_back(std::move(buffer));
    }
    std::vector<std::function<void()>>& GetDeferredDeleters() { return DeferredDeleters; }
    void AddWaitSemaphore(VkSemaphore sem, VkPipelineStageFlags stages)
    {
        WaitSemaphores.push_back(sem);
        WaitStages.push_back(stages);
    }
    VkSemaphore GetSignalSemaphore(VkDevice device, bool create = false)
    {
        // Create one if doesn't have it
        VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        if (!SignalSemaphore && create)
            vkCreateSemaphore(device, &createInfo, nullptr, &SignalSemaphore);
        return SignalSemaphore;
    }
    void Submit(CSubmissionTracker& tracker, std::unique_ptr<CCommandBufferVk> cmdBufferPre);

    CRenderPass::Ref GetRenderPass() const { return RenderPass; }
    void AppendSubpassContent(uint32_t subpass, std::unique_ptr<CCommandBufferVk> cmdBuffer)
    {
        SubpassContents[subpass].push_back(std::move(cmdBuffer));
    }

private:
    friend class CRenderPassContextVk;
    friend class CCommandContextVk;

    bool bCurrentlyRecording = false;
    CAccessTracker AccessTracker;
    std::vector<std::unique_ptr<CCommandBufferVk>> CmdBuffers;

    std::vector<std::function<void()>> DeferredDeleters;
    std::vector<VkSemaphore> WaitSemaphores;
    std::vector<VkPipelineStageFlags> WaitStages;
    VkSemaphore SignalSemaphore = VK_NULL_HANDLE;

    // Temporary data for recording render pass
    CRenderPass::Ref RenderPass;
    std::array<std::vector<std::unique_ptr<CCommandBufferVk>>, 4>
        SubpassContents; // only 4 total subpasses?
};

class CDeferredContextVk : public IRenderContext
{
    static void Convert(VkOffset3D& dst, const COffset3D& src);
    static void Convert(VkExtent3D& dst, const CExtent3D& src);
    static void Convert(VkImageSubresourceLayers& dst, const CImageSubresourceLayers& src);
    static void Convert(VkImageCopy& dst, const CImageCopy& src);
    static void Convert(VkImageResolve& dst, const CImageResolve& src);
    static void Convert(VkBufferImageCopy& dst, const CBufferImageCopy& src);
    static void Convert(VkImageBlit& dst, const CImageBlit& src);
    void TransitionImage(CImage& image, EResourceState newState);
    void ResolveBindings();

public:
    typedef std::shared_ptr<CDeferredContextVk> Ref;

    CDeferredContextVk(CDeviceVk& p, CCommandListVk::Ref cmdList);
    CDeferredContextVk(CDeviceVk& p, CCommandListVk::Ref cmdList, uint32_t subpass);
    ~CDeferredContextVk();

    void CopyBuffer(CBuffer& src, CBuffer& dst, const std::vector<CBufferCopy>& regions) override;
    void CopyImage(CImage& src, CImage& dst, const std::vector<CImageCopy>& regions) override;
    void CopyBufferToImage(CBuffer& src, CImage& dst,
                           const std::vector<CBufferImageCopy>& regions) override;
    void CopyImageToBuffer(CImage& src, CBuffer& dst,
                           const std::vector<CBufferImageCopy>& regions) override;
    void BlitImage(CImage& src, CImage& dst, const std::vector<CImageBlit>& regions,
                   EFilter filter) override;
    void ResolveImage(CImage& src, CImage& dst, const std::vector<CImageResolve>& regions) override;
    void FinishRecording() override;
    void BindPipeline(CPipeline& pipeline) override;
    void BindBuffer(CBuffer& buffer, size_t offset, size_t range, uint32_t set, uint32_t binding,
                    uint32_t index) override;
    void BindBufferView(CBufferView& bufferView, uint32_t set, uint32_t binding,
                        uint32_t index) override;
    void BindConstants(const void* pData, size_t size, uint32_t set, uint32_t binding,
                       uint32_t index) override;
    void BindImageView(CImageView& imageView, uint32_t set, uint32_t binding,
                       uint32_t index) override;
    void BindSampler(CSampler& sampler, uint32_t set, uint32_t binding, uint32_t index) override;
    void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
    void BindIndexBuffer(CBuffer& buffer, size_t offset, EFormat format) override;
    void BindVertexBuffer(uint32_t binding, CBuffer& buffer, size_t offset) override;
    void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
              uint32_t firstInstance) override;
    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                     int32_t vertexOffset, uint32_t firstInstance) override;

private:
    CDeviceVk& Parent;
    CCommandListVk::Ref CommandList;
    std::unique_ptr<CCommandBufferVk> CmdBuffer;
    bool bIsRenderPassContent = false;
    uint32_t Subpass = 0;

    CPipelineVk* CurrPipeline = nullptr;
    ResourceBindings CurrBindings;
    std::map<uint32_t, CDescriptorSetLayoutVk*> BoundDescriptorSetLayouts;
};

class CRenderPassContextVk : public IRenderPassContext
{
public:
    typedef std::shared_ptr<CDeferredContextVk> Ref;

    CRenderPassContextVk(CDeviceVk& p, CCommandListVk::Ref cmdList, CRenderPass& renderPass,
                         const std::vector<CClearValue>& clearValues);

    IRenderContext::Ref CreateRenderContext(uint32_t subpass) override;
    void FinishRecording() override;

private:
    CDeviceVk& Parent;
    CCommandListVk::Ref CommandList;
    const std::vector<CClearValue>& ClearValues;
};

} /* namespace RHI */
