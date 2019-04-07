#include "CommandContextVk.h"
#include "DeviceVk.h"
#include "PipelineVk.h"
#include "RenderPassVk.h"
#include "VkHelpers.h"

namespace RHI
{

class CCommandListVk : public CCommandList
{
public:
    bool bIsSecondary = false;
    VkCommandBuffer CmdBuffer;
    CCommandContextVk::Ref Parent;
};

CCommandContextVk::CCommandContextVk(CDeviceVk& p, uint32_t qfi, bool deferredContext)
    : Parent(p)
    , QueueType(qfi)
    , bIsDeferred(deferredContext)
{
    VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    if (bIsDeferred)
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    else
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = Parent.GetQueueFamily(QueueType);
    vkCreateCommandPool(Parent.GetVkDevice(), &poolInfo, nullptr, &CmdPool);

    BeginBuffer();
}

CCommandContextVk::~CCommandContextVk()
{
    EndBuffer();
    vkDestroySemaphore(Parent.GetVkDevice(), SignalSemaphore, nullptr);
    vkFreeCommandBuffers(Parent.GetVkDevice(), CmdPool, 1, &CmdBuffer);
    std::unique_lock<std::mutex> lk(GarbageMutex);
    vkFreeCommandBuffers(Parent.GetVkDevice(), CmdPool, GarbageBuffers.size(),
                         GarbageBuffers.data());
    GarbageBuffers.clear();
    vkDestroyCommandPool(Parent.GetVkDevice(), CmdPool, nullptr);
}

void CCommandContextVk::BeginBuffer()
{
    // Clear garbage
    std::unique_lock<std::mutex> lk(GarbageMutex);
    vkFreeCommandBuffers(Parent.GetVkDevice(), CmdPool, GarbageBuffers.size(),
                         GarbageBuffers.data());
    GarbageBuffers.clear();

    // Create a new command buffer
    VkCommandBufferAllocateInfo cmdInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdInfo.commandPool = CmdPool;
    cmdInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(Parent.GetVkDevice(), &cmdInfo, &CmdBuffer);

    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    if (!bIsDeferred)
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(CmdBuffer, &beginInfo);

    WaitSemaphores.clear();
    WaitStages.clear();
	VkSemaphoreCreateInfo semaphoreInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    vkCreateSemaphore(Parent.GetVkDevice(), &semaphoreInfo, nullptr, &SignalSemaphore);
}

void CCommandContextVk::EndBuffer() { vkEndCommandBuffer(CmdBuffer); }

void CCommandContextVk::TransitionImage(CImage* image, EResourceState newState)
{
    auto* imageImpl = static_cast<CImageVk*>(image);
    VkImageLayout srcLayout = StateToImageLayout(imageImpl->GetGlobalState());
    VkImageLayout dstLayout = StateToImageLayout(newState);

    if (srcLayout != dstLayout)
    {
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = srcLayout;
        barrier.newLayout = dstLayout;
        barrier.image = imageImpl->Image;
        barrier.subresourceRange.aspectMask =
            GetImageAspectFlags(imageImpl->GetCreateInfo().format);
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.layerCount = imageImpl->GetCreateInfo().arrayLayers;
        barrier.subresourceRange.levelCount = imageImpl->GetCreateInfo().mipLevels;
        barrier.srcAccessMask = StateToAccessMask(imageImpl->GetGlobalState());
        barrier.dstAccessMask = StateToAccessMask(newState);

        VkPipelineStageFlags srcStageMask =
            StateToShaderStageMask(imageImpl->GetGlobalState(), true);
        VkPipelineStageFlags dstStageMask = StateToShaderStageMask(newState, false);
        vkCmdPipelineBarrier(CmdBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1,
                             &barrier);

        imageImpl->SetGlobalState(newState);
    }
}

void CCommandContextVk::CopyBuffer(CBuffer* src, CBuffer* dst,
                                   const std::vector<CBufferCopy>& regions)
{
    static_assert(sizeof(CBufferCopy) == sizeof(VkBufferCopy), "struct size mismatch");
    const VkBufferCopy* r = reinterpret_cast<const VkBufferCopy*>(regions.data());

    vkCmdCopyBuffer(CmdBuffer, static_cast<CBufferVk*>(src)->Buffer,
                    static_cast<CBufferVk*>(dst)->Buffer, regions.size(), r);
}

void CCommandContextVk::CopyImage(CImage* src, CImage* dst, const std::vector<CImageCopy>& regions)
{
    std::vector<VkImageCopy> r;
    for (const auto& rs : regions)
    {
        VkImageCopy next;
        Convert(next, rs);
        r.push_back(next);
    }
    TransitionImage(src, EResourceState::CopySource);
    TransitionImage(dst, EResourceState::CopyDest);
    auto* srcImpl = static_cast<CImageVk*>(src);
    auto* dstImpl = static_cast<CImageVk*>(dst);
    vkCmdCopyImage(CmdBuffer, srcImpl->Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImpl->Image,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, r.size(), r.data());
}

void CCommandContextVk::CopyBufferToImage(CBuffer* src, CImage* dst,
                                          const std::vector<CBufferImageCopy>& regions)
{
    std::vector<VkBufferImageCopy> vkregions;
    for (const auto& rs : regions)
    {
        VkBufferImageCopy next;
        Convert(next, rs);
        vkregions.push_back(next);
    }
    TransitionImage(dst, EResourceState::CopyDest);
    auto* dstImpl = static_cast<CImageVk*>(dst);
    vkCmdCopyBufferToImage(CmdBuffer, static_cast<CBufferVk*>(src)->Buffer, dstImpl->Image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vkregions.size(),
                           vkregions.data());
}

void CCommandContextVk::CopyImageToBuffer(CImage* src, CBuffer* dst,
                                          const std::vector<CBufferImageCopy>& regions)
{
    std::vector<VkBufferImageCopy> vkregions;
    for (const auto& rs : regions)
    {
        VkBufferImageCopy next;
        Convert(next, rs);
        vkregions.push_back(next);
    }
    TransitionImage(src, EResourceState::CopySource);
    auto* srcImpl = static_cast<CImageVk*>(src);
    vkCmdCopyImageToBuffer(CmdBuffer, srcImpl->Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           static_cast<CBufferVk*>(dst)->Buffer, vkregions.size(),
                           vkregions.data());
}

void CCommandContextVk::BlitImage(CImage* src, CImage* dst, const std::vector<CImageBlit>& regions,
                                  EFilter filter)
{
    std::vector<VkImageBlit> r;
    for (const auto& rs : regions)
    {
        VkImageBlit next;
        Convert(next, rs);
        r.push_back(next);
    }
    TransitionImage(src, EResourceState::CopySource);
    TransitionImage(dst, EResourceState::CopyDest);
    auto* srcImpl = static_cast<CImageVk*>(src);
    auto* dstImpl = static_cast<CImageVk*>(dst);
    vkCmdBlitImage(CmdBuffer, srcImpl->Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImpl->Image,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, r.size(), r.data(), VkCast(filter));
}

void CCommandContextVk::ResolveImage(CImage* src, CImage* dst,
                                     const std::vector<CImageResolve>& regions)
{
    std::vector<VkImageResolve> r;
    for (const auto& rs : regions)
    {
        VkImageResolve next;
        Convert(next, rs);
        r.push_back(next);
    }
    TransitionImage(src, EResourceState::CopySource);
    TransitionImage(dst, EResourceState::CopyDest);
    auto* srcImpl = static_cast<CImageVk*>(src);
    auto* dstImpl = static_cast<CImageVk*>(dst);
    vkCmdResolveImage(CmdBuffer, srcImpl->Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                      dstImpl->Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, r.size(), r.data());
}

void CCommandContextVk::ExecuteCommandList(CCommandList* commandList)
{
    auto* cmdListImpl = static_cast<CCommandListVk*>(commandList);
    if (cmdListImpl->bIsSecondary)
    {
        vkCmdExecuteCommands(CmdBuffer, 1, &cmdListImpl->CmdBuffer);
    }
    else
    {
        Flush();
        CGPUJobInfo job;
        job.QueueType = static_cast<EQueueType>(QueueType);
        job.AddCommandBuffer(cmdListImpl->CmdBuffer, cmdListImpl->Parent);
        Parent.SubmitJob(std::move(job));
    }
}

CCommandList::Ref CCommandContextVk::FinishCommandList()
{
    EndBuffer();
    auto ptr = std::make_shared<CCommandListVk>();
    ptr->CmdBuffer = CmdBuffer;
    ptr->bIsSecondary = false;
    ptr->Parent = this->shared_from_this();
    BeginBuffer();
    return ptr;
}

void CCommandContextVk::Flush(bool wait)
{
    EndBuffer();

    if (wait)
    {
        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &CmdBuffer;
        submitInfo.waitSemaphoreCount = static_cast<uint32_t>(WaitSemaphores.size());
        submitInfo.pWaitSemaphores = WaitSemaphores.data();
        submitInfo.pWaitDstStageMask = WaitStages.data();
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &SignalSemaphore;

        VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        VkFence fence;
        vkCreateFence(Parent.GetVkDevice(), &fenceInfo, nullptr, &fence);

        VkQueue q = Parent.GetVkQueue(QueueType);
        vkQueueSubmit(q, 1, &submitInfo, fence);
        vkWaitForFences(Parent.GetVkDevice(), 1, &fence, VK_TRUE,
                        std::numeric_limits<uint64_t>::max());
        vkDestroyFence(Parent.GetVkDevice(), fence, nullptr);
        vkDestroySemaphore(Parent.GetVkDevice(), SignalSemaphore, nullptr);

        DoneWithCmdBuffer(CmdBuffer);
    }
    else
    {
        // Bye bye command buffer, I hope you all well on the GPU side
        CGPUJobInfo job;
        job.QueueType = static_cast<EQueueType>(QueueType);
        job.AddCommandBuffer(CmdBuffer, this->shared_from_this());
        job.WaitSemaphores = WaitSemaphores;
        job.WaitStages = WaitStages;
        job.SignalSemaphore = SignalSemaphore;
        Parent.SubmitJob(std::move(job));
    }

    BeginBuffer();
}

void CCommandContextVk::BeginRenderPass(CRenderPass::Ref renderPass,
                                        const std::vector<CClearValue>& clearValues)
{
    auto rpImpl = std::static_pointer_cast<CRenderPassVk>(renderPass);
    auto framebufferInfo = rpImpl->GetNextFramebuffer();
    VkRenderPassBeginInfo beginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    beginInfo.renderPass = rpImpl->RenderPass;
    beginInfo.framebuffer = framebufferInfo.first;
    if (framebufferInfo.second != VK_NULL_HANDLE)
    {
        WaitSemaphores.emplace_back(rpImpl->GetNextFramebuffer().second);
        WaitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	}
    std::vector<VkClearValue> clear;
    size_t i = 0;
    for (const auto& c : clearValues)
    {
        VkClearValue vkClear;
        auto aspect = GetImageAspectFlags(rpImpl->GetAttachmentDesc()[i].format);
        if (aspect & VK_IMAGE_ASPECT_COLOR_BIT)
            memcpy(vkClear.color.int32, c.ColorInt32, sizeof(vkClear.color));
        else
        {
            vkClear.depthStencil.depth = c.Depth;
            vkClear.depthStencil.stencil = c.Stencil;
        }
        clear.push_back(vkClear);
        i++;
    }
    beginInfo.clearValueCount = static_cast<uint32_t>(clear.size());
    beginInfo.pClearValues = clear.data();
    RenderArea = beginInfo.renderArea = rpImpl->GetArea();

    vkCmdBeginRenderPass(CmdBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CCommandContextVk::NextSubpass() { throw "unimplemented"; }

void CCommandContextVk::EndRenderPass() { vkCmdEndRenderPass(CmdBuffer); }

void CCommandContextVk::BindPipeline(CPipeline* pipeline)
{
    auto* pipelineImpl = static_cast<CPipelineVk*>(pipeline);
    vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineImpl->GetHandle());

    // Bind the default viewport and scissors
    VkViewport vp;
    vp.x = 0;
    vp.y = 0;
    vp.width = RenderArea.extent.width;
    vp.height = RenderArea.extent.height;
    vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;
    vkCmdSetViewport(CmdBuffer, 0, 1, &vp);
    vkCmdSetScissor(CmdBuffer, 0, 1, &RenderArea);
    const float blendFactor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    vkCmdSetBlendConstants(CmdBuffer, blendFactor);
    vkCmdSetStencilReference(CmdBuffer, VK_STENCIL_FRONT_AND_BACK, 0);
}

void CCommandContextVk::BindBuffer(CBuffer* buffer, size_t offset, size_t range, uint32_t set,
                                   uint32_t binding, uint32_t index)
{
    throw "unimplemented";
}

void CCommandContextVk::BindBufferView(CBufferView* bufferView, uint32_t set, uint32_t binding,
                                       uint32_t index)
{
    throw "unimplemented";
}

void CCommandContextVk::BindImageView(CImageView* imageView, uint32_t set, uint32_t binding,
                                      uint32_t index)
{
    throw "unimplemented";
}

void CCommandContextVk::BindSampler(CSampler* sampler, uint32_t set, uint32_t binding,
                                    uint32_t index)
{
    throw "unimplemented";
}

void CCommandContextVk::BindIndexBuffer(CBuffer* buffer, size_t offset, EFormat format)
{
    throw "unimplemented";
}

void CCommandContextVk::BindVertexBuffer(uint32_t binding, CBuffer* buffer, size_t offset)
{
    throw "unimplemented";
}

void CCommandContextVk::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                             uint32_t firstInstance)
{
    // TODO: resolve bindings
    vkCmdDraw(CmdBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CCommandContextVk::DrawIndexed(uint32_t indexCount, uint32_t instanceCount,
                                    uint32_t firstIndex, int32_t vertexOffset,
                                    uint32_t firstInstance)
{
    // TODO: resolve bindings
    vkCmdDrawIndexed(CmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CCommandContextVk::DoneWithCmdBuffer(VkCommandBuffer b)
{
    std::unique_lock<std::mutex> lk(GarbageMutex);

    GarbageBuffers.push_back(b);
}

void CCommandContextVk::Convert(VkOffset3D& dst, const COffset3D& src)
{
    static_assert(sizeof(VkOffset3D) == sizeof(COffset3D), "struct size mismatch");
    dst = reinterpret_cast<const VkOffset3D&>(src);
}

void CCommandContextVk::Convert(VkExtent3D& dst, const CExtent3D& src)
{
    static_assert(sizeof(VkExtent3D) == sizeof(CExtent3D), "struct size mismatch");
    dst = reinterpret_cast<const VkExtent3D&>(src);
}

void CCommandContextVk::Convert(VkImageSubresourceLayers& dst, const CImageSubresourceLayers& src)
{
    dst.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT; // TODO:
    dst.baseArrayLayer = src.BaseArrayLayer;
    dst.layerCount = src.LayerCount;
    dst.mipLevel = src.MipLevel;
}

void CCommandContextVk::Convert(VkImageCopy& dst, const CImageCopy& src)
{
    Convert(dst.srcSubresource, src.SrcSubresource);
    Convert(dst.srcOffset, src.SrcOffset);
    Convert(dst.dstSubresource, src.DstSubresource);
    Convert(dst.dstOffset, src.DstOffset);
    Convert(dst.extent, src.Extent);
}

void CCommandContextVk::Convert(VkImageResolve& dst, const CImageResolve& src)
{
    Convert(dst.srcSubresource, src.SrcSubresource);
    Convert(dst.srcOffset, src.SrcOffset);
    Convert(dst.dstSubresource, src.DstSubresource);
    Convert(dst.dstOffset, src.DstOffset);
    Convert(dst.extent, src.Extent);
}

void CCommandContextVk::Convert(VkBufferImageCopy& dst, const CBufferImageCopy& src)
{
    dst.bufferOffset = src.BufferOffset;
    dst.bufferRowLength = src.BufferRowLength;
    dst.bufferImageHeight = src.BufferImageHeight;
    Convert(dst.imageSubresource, src.ImageSubresource);
    Convert(dst.imageOffset, src.ImageOffset);
    Convert(dst.imageExtent, src.ImageExtent);
}

void CCommandContextVk::Convert(VkImageBlit& dst, const CImageBlit& src)
{
    Convert(dst.srcSubresource, src.SrcSubresource);
    Convert(dst.srcOffsets[0], src.SrcOffsets[0]);
    Convert(dst.srcOffsets[1], src.SrcOffsets[1]);
    Convert(dst.dstSubresource, src.DstSubresource);
    Convert(dst.dstOffsets[0], src.DstOffsets[0]);
    Convert(dst.dstOffsets[1], src.DstOffsets[1]);
}

}
