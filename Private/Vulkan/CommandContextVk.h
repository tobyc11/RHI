#pragma once
#include "BufferVk.h"
#include "CommandBufferVk.h"
#include "CopyContext.h"
#include "ImageVk.h"
#include "PipelineVk.h"
#include "RenderContext.h"
#include "ResourceBindingsVk.h"
#include "SubmissionTracker.h"
#include "VkCommon.h"
#include <memory>
#include <mutex>
#include <vector>

namespace RHI
{

class CRenderPassVk;

// A general command buffer recorder
class CCommandContextVk : public std::enable_shared_from_this<CCommandContextVk>,
                          public IImmediateContext
{
    static void Convert(VkOffset3D& dst, const COffset3D& src);
    static void Convert(VkExtent3D& dst, const CExtent3D& src);
    static void Convert(VkImageSubresourceLayers& dst, const CImageSubresourceLayers& src);
    static void Convert(VkImageCopy& dst, const CImageCopy& src);
    static void Convert(VkImageResolve& dst, const CImageResolve& src);
    static void Convert(VkBufferImageCopy& dst, const CBufferImageCopy& src);
    static void Convert(VkImageBlit& dst, const CImageBlit& src);

public:
    typedef std::shared_ptr<CCommandContextVk> Ref;

    CCommandContextVk(CDeviceVk& p, EQueueType queueType, ECommandContextKind kind);
    virtual ~CCommandContextVk();

    void BeginBuffer();
    void EndBuffer();

    // For internal use only
    VkCommandBuffer GetBuffer() const { return CmdBuffer->GetHandle(); }
    VkSemaphore GetSignalSemaphore() const { return SignalSemaphore; }
    bool IsInRenderPass() const { return CurrRenderPass; }

    void TransitionImage(CImage& image, EResourceState newState);

    // Copy commands
    void CopyBuffer(CBuffer& src, CBuffer& dst, const std::vector<CBufferCopy>& regions);
    void CopyImage(CImage& src, CImage& dst, const std::vector<CImageCopy>& regions);
    void CopyBufferToImage(CBuffer& src, CImage& dst, const std::vector<CBufferImageCopy>& regions);
    void CopyImageToBuffer(CImage& src, CBuffer& dst, const std::vector<CBufferImageCopy>& regions);
    void BlitImage(CImage& src, CImage& dst, const std::vector<CImageBlit>& regions,
                   EFilter filter);
    void ResolveImage(CImage& src, CImage& dst, const std::vector<CImageResolve>& regions);
    void FinishRecording();

    // Compute/Render commands
    void BindPipeline(CPipeline& pipeline);
    void BindBuffer(CBuffer& buffer, size_t offset, size_t range, uint32_t set, uint32_t binding,
                    uint32_t index);
    void BindBufferView(CBufferView& bufferView, uint32_t set, uint32_t binding, uint32_t index);
    void BindConstants(const void* pData, size_t size, uint32_t set, uint32_t binding,
                       uint32_t index);
    void BindImageView(CImageView& imageView, uint32_t set, uint32_t binding, uint32_t index);
    void BindSampler(CSampler& sampler, uint32_t set, uint32_t binding, uint32_t index);
    void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

    // Render commands
    void BindIndexBuffer(CBuffer& buffer, size_t offset, EFormat format);
    void BindVertexBuffer(uint32_t binding, CBuffer& buffer, size_t offset);
    void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
              uint32_t firstInstance);
    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                     int32_t vertexOffset, uint32_t firstInstance);

    // Immediate context commands
    void ExecuteCommandList(CCommandList& commandList);
    void Flush(bool wait = false) { Flush(wait, false); }
    void Flush(bool wait, bool isPresent);
    void BeginRenderPass(CRenderPass& renderPass, const std::vector<CClearValue>& clearValues);
    void NextSubpass();
    void EndRenderPass();

private:
    void ResolveBindings();

private:
    CDeviceVk& Parent;
    ECommandContextKind Kind;
    EQueueType QueueType;

    std::unique_ptr<CCommandBufferVk> CmdBuffer;

    // Render states kept track of
    CAccessTracker AccessTracker;
    CRenderPassVk* CurrRenderPass = nullptr;

    CPipelineVk* CurrPipeline = nullptr;
    ResourceBindings CurrBindings;
    std::map<uint32_t, CDescriptorSetLayoutVk*> BoundDescriptorSetLayouts;
    std::vector<std::function<void()>> DeferredDeleters;
    std::vector<VkSemaphore> WaitSemaphores;
    std::vector<VkPipelineStageFlags> WaitStages;
    VkSemaphore SignalSemaphore = VK_NULL_HANDLE;
};

} /* namespace RHI */
