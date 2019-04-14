#include "CommandContextVk.h"
#include "DeviceVk.h"
#include "PipelineVk.h"
#include "RenderPassVk.h"
#include "SamplerVk.h"
#include "VkHelpers.h"
#include <unordered_set>

namespace RHI
{

class CCommandListVk : public CCommandList
{
public:
    bool bIsSecondary = false;
    VkCommandBuffer CmdBuffer = VK_NULL_HANDLE;
    CCommandContextVk::Ref Parent;
};

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
    vkFreeCommandBuffers(Parent.GetVkDevice(), CmdPool,
                         static_cast<uint32_t>(GarbageBuffers.size()), GarbageBuffers.data());
    GarbageBuffers.clear();
    vkDestroyCommandPool(Parent.GetVkDevice(), CmdPool, nullptr);
}

void CCommandContextVk::BeginBuffer()
{
    // Clear garbage
    std::unique_lock<std::mutex> lk(GarbageMutex);
    vkFreeCommandBuffers(Parent.GetVkDevice(), CmdPool,
                         static_cast<uint32_t>(GarbageBuffers.size()), GarbageBuffers.data());
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

    DeferredDeleters.clear();
}

void CCommandContextVk::EndBuffer()
{
    vkEndCommandBuffer(CmdBuffer);
    CurrBindings.Reset();
}

void CCommandContextVk::TransitionImage(CImage* image, EResourceState newState)
{
    auto* imageImpl = static_cast<CImageVk*>(image);
    VkImageLayout srcLayout = StateToImageLayout(imageImpl->GetGlobalState());
    VkImageLayout dstLayout = StateToImageLayout(newState);

    if (srcLayout != dstLayout)
    {
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = StateToAccessMask(imageImpl->GetGlobalState());
        barrier.dstAccessMask = StateToAccessMask(newState);
        barrier.oldLayout = srcLayout;
        barrier.newLayout = dstLayout;
        barrier.srcQueueFamilyIndex = 0;
        barrier.dstQueueFamilyIndex = 0;
        if (imageImpl->IsConcurrentAccess())
            barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = imageImpl->GetVkImage();
        barrier.subresourceRange.aspectMask =
            GetImageAspectFlags(imageImpl->GetCreateInfo().format);
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.layerCount = imageImpl->GetCreateInfo().arrayLayers;
        barrier.subresourceRange.levelCount = imageImpl->GetCreateInfo().mipLevels;

        VkPipelineStageFlags srcStageMask =
            StateToShaderStageMask(imageImpl->GetGlobalState(), true);
        VkPipelineStageFlags dstStageMask = StateToShaderStageMask(newState, false);
        if (QueueType == TransferQueue)
        {
            dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            barrier.dstAccessMask = 0;
        }
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
                    static_cast<CBufferVk*>(dst)->Buffer, static_cast<uint32_t>(regions.size()), r);
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
    vkCmdCopyImage(CmdBuffer, srcImpl->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   dstImpl->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   static_cast<uint32_t>(r.size()), r.data());
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
    vkCmdCopyBufferToImage(CmdBuffer, static_cast<CBufferVk*>(src)->Buffer, dstImpl->GetVkImage(),
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<uint32_t>(vkregions.size()), vkregions.data());
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
    vkCmdCopyImageToBuffer(CmdBuffer, srcImpl->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           static_cast<CBufferVk*>(dst)->Buffer,
                           static_cast<uint32_t>(vkregions.size()), vkregions.data());
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
    vkCmdBlitImage(CmdBuffer, srcImpl->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   dstImpl->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   static_cast<uint32_t>(r.size()), r.data(), VkCast(filter));
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
    vkCmdResolveImage(CmdBuffer, srcImpl->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                      dstImpl->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                      static_cast<uint32_t>(r.size()), r.data());
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
        for (auto& fn : DeferredDeleters)
            fn();

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
        job.DeferredDeleters = DeferredDeleters;
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
    bIsInRenderPass = true;

    vkCmdBeginRenderPass(CmdBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CCommandContextVk::NextSubpass() { throw "unimplemented"; }

void CCommandContextVk::EndRenderPass()
{
    bIsInRenderPass = false;
    vkCmdEndRenderPass(CmdBuffer);
}

void CCommandContextVk::BindPipeline(CPipeline::Ref pipeline)
{
    CurrPipeline = static_cast<CPipelineVk*>(pipeline.get());
    vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, CurrPipeline->GetHandle());

    // Bind the default viewport and scissors
    VkViewport vp;
    vp.x = 0;
    vp.y = 0;
    vp.width = static_cast<float>(RenderArea.extent.width);
    vp.height = static_cast<float>(RenderArea.extent.height);
    vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;
    vkCmdSetViewport(CmdBuffer, 0, 1, &vp);
    vkCmdSetScissor(CmdBuffer, 0, 1, &RenderArea);
    const float blendFactor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    vkCmdSetBlendConstants(CmdBuffer, blendFactor);
    vkCmdSetStencilReference(CmdBuffer, VK_STENCIL_FRONT_AND_BACK, 0);
}

void CCommandContextVk::BindBuffer(CBuffer::Ref buffer, size_t offset, size_t range, uint32_t set,
                                   uint32_t binding, uint32_t index)
{
    auto bufferImpl = std::static_pointer_cast<CBufferVk>(buffer);
    CurrBindings.BindBuffer(bufferImpl, offset, range, set, binding, index);
}

void CCommandContextVk::BindBufferView(CBufferView::Ref bufferView, uint32_t set, uint32_t binding,
                                       uint32_t index)
{
    throw "unimplemented";
}

void CCommandContextVk::BindImageView(CImageView::Ref imageView, uint32_t set, uint32_t binding,
                                      uint32_t index)
{
    auto impl = std::static_pointer_cast<CImageViewVk>(imageView);
    assert(impl->GetImage()->GetGlobalState() == EResourceState::ShaderResource);
    CurrBindings.BindImageView(impl, VK_NULL_HANDLE, set, binding, index);
}

void CCommandContextVk::BindSampler(CSampler::Ref sampler, uint32_t set, uint32_t binding,
                                    uint32_t index)
{
    CurrBindings.BindSampler(static_cast<CSamplerVk*>(sampler.get())->Sampler, set, binding, index);
}

void CCommandContextVk::BindIndexBuffer(CBuffer::Ref buffer, size_t offset, EFormat format)
{
    auto bufferImpl = std::static_pointer_cast<CBufferVk>(buffer);
    vkCmdBindIndexBuffer(CmdBuffer, bufferImpl->Buffer, offset,
                         format == EFormat::R16_UINT ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
}

void CCommandContextVk::BindVertexBuffer(uint32_t binding, CBuffer::Ref buffer, size_t offset)
{
    auto bufferImpl = std::static_pointer_cast<CBufferVk>(buffer);
    // On macOS, size_t is 32 bit and VkDeviceSize is 64
    VkDeviceSize offsetDevice = offset;
    vkCmdBindVertexBuffers(CmdBuffer, binding, 1, &bufferImpl->Buffer, &offsetDevice);
}

void CCommandContextVk::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                             uint32_t firstInstance)
{
    ResolveBindings();
    vkCmdDraw(CmdBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CCommandContextVk::DrawIndexed(uint32_t indexCount, uint32_t instanceCount,
                                    uint32_t firstIndex, int32_t vertexOffset,
                                    uint32_t firstInstance)
{
    ResolveBindings();
    vkCmdDrawIndexed(CmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CCommandContextVk::DoneWithCmdBuffer(VkCommandBuffer b)
{
    std::unique_lock<std::mutex> lk(GarbageMutex);

    GarbageBuffers.push_back(b);
}

void CCommandContextVk::ResolveBindings()
{
    if (CurrPipeline)
    {
        std::unordered_set<uint32_t> setConflicts;
        const auto& pipelineBindings = CurrPipeline->GetSetBindings();
        for (auto it : pipelineBindings)
        {
            // For given set index, if descriptor set layouts differ, store set index in conflicts
            // map.
            auto set = it.first;
            if (BoundDescriptorSetLayouts.find(set) != BoundDescriptorSetLayouts.end())
            {
                if (BoundDescriptorSetLayouts.at(set) != CurrPipeline->GetSetLayout(set))
                    setConflicts.emplace(set);
            }
        }

        // Remove entries from boundDescriptorSetLayouts that do not exist in pipeline.
        auto it = BoundDescriptorSetLayouts.begin();
        while (it != BoundDescriptorSetLayouts.end())
        {
            if (!CurrPipeline->GetSetLayout(it->first))
                it = BoundDescriptorSetLayouts.erase(it);
            else
                ++it;
        }

        // Evaluate whether a new descriptor set must be created and bound.
        if (CurrBindings.IsDirty() || !setConflicts.empty())
        {
            // Reset resource bindings dirty bit.
            CurrBindings.ClearDirtyBit();

            // Iterate over each set binding.
            for (auto setBindingsItr : CurrBindings.GetSetBindings())
            {
                // Skip if no bindings having changes.
                auto set = setBindingsItr.first;
                auto& setBindings = setBindingsItr.second;
                if (!setBindings.bDirty && (setConflicts.find(set) == setConflicts.end()))
                    continue;

                // Reset set binding dirty flag.
                setBindings.bDirty = false;

                // Retrieve the descriptor set layout for the given set index.
                // If set index is not used with bound pipeline, skip it.
                auto descriptorSetLayout = CurrPipeline->GetSetLayout(set);
                if (!descriptorSetLayout)
                    continue;

                // Allocate a new descriptor set. (TODO! Log or report error if allocation fails)
                auto descriptorSet = descriptorSetLayout->AllocateDescriptorSet();
                if (!descriptorSet)
                    continue;

                // Set descriptor set layout as active for given set index.
                BoundDescriptorSetLayouts[set] = descriptorSetLayout;

                // Update all of the set's bindings.
                std::vector<VkDescriptorBufferInfo> bufferInfos;
                std::vector<VkDescriptorImageInfo> imageInfos;
                std::vector<VkWriteDescriptorSet> descriptorWrites;
                for (auto bindingItr : setBindings.Bindings)
                {
                    // Get layout binding for given binding index.
                    auto binding = bindingItr.first;
                    VkDescriptorSetLayoutBinding* layoutBinding = nullptr;
                    if (!descriptorSetLayout->GetLayoutBinding(binding, &layoutBinding))
                        continue;

                    // Determine binding's access and pipeline stage flags.
                    auto shaderStages = static_cast<VkShaderStageFlags>(layoutBinding->stageFlags);

                    VkPipelineStageFlags stageMask = 0;
                    if (shaderStages & VK_SHADER_STAGE_VERTEX_BIT)
                        stageMask |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                    if (shaderStages & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)
                        stageMask |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
                    if (shaderStages & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
                        stageMask |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
                    if (shaderStages & VK_SHADER_STAGE_GEOMETRY_BIT)
                        stageMask |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
                    if (shaderStages & VK_SHADER_STAGE_FRAGMENT_BIT)
                        stageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                    if (shaderStages & VK_SHADER_STAGE_COMPUTE_BIT)
                        stageMask |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

                    // Iterate over all array elements.
                    auto& arrayElementBindings = bindingItr.second;
                    for (auto arrayElementItr : arrayElementBindings)
                    {
                        // Get the binding info.
                        auto arrayElement = arrayElementItr.first;
                        auto& bindingInfo = arrayElementItr.second;

                        // Fill in descriptor set write structure.
                        VkWriteDescriptorSet dsWrite = {};
                        dsWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        dsWrite.dstSet = descriptorSet;
                        dsWrite.dstBinding = binding;
                        dsWrite.dstArrayElement = arrayElement;
                        dsWrite.descriptorType = layoutBinding->descriptorType;
                        dsWrite.descriptorCount = 1;

                        // Handle buffers.
                        if (bindingInfo.pBuffer)
                        {
                            VkDescriptorBufferInfo bufferInfo = {};
                            bufferInfo.buffer = bindingInfo.pBuffer->Buffer;
                            bufferInfo.offset = bindingInfo.offset;
                            bufferInfo.range = bindingInfo.range;
                            bufferInfos.push_back(bufferInfo);
                            dsWrite.pBufferInfo =
                                reinterpret_cast<const VkDescriptorBufferInfo*>(bufferInfos.size());
                        }
                        // Handle images and samplers.
                        else if (bindingInfo.pImageView || bindingInfo.sampler != VK_NULL_HANDLE)
                        {
                            VkDescriptorImageInfo imageInfo = {};

                            if (bindingInfo.sampler != VK_NULL_HANDLE)
                            {
                                imageInfo.sampler =
                                    reinterpret_cast<VkSampler>(bindingInfo.sampler);
                            }

                            if (bindingInfo.pImageView)
                            {
                                imageInfo.imageView = bindingInfo.pImageView->GetVkImageView();
                                imageInfo.imageLayout =
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // TODO: fixme
                            }

                            imageInfos.push_back(imageInfo);
                            dsWrite.pImageInfo =
                                reinterpret_cast<const VkDescriptorImageInfo*>(imageInfos.size());
                        }

                        // Add the descriptor set write to the list.
                        descriptorWrites.push_back(dsWrite);
                    }
                }

                // Iterate back over descriptorWrites array and fix pointer addresses.
                for (auto& dswrite : descriptorWrites)
                {
                    if (dswrite.pBufferInfo)
                        dswrite.pBufferInfo =
                            &bufferInfos[reinterpret_cast<uint64_t>(dswrite.pBufferInfo) - 1];
                    else if (dswrite.pImageInfo)
                        dswrite.pImageInfo =
                            &imageInfos[reinterpret_cast<uint64_t>(dswrite.pImageInfo) - 1];
                }

                // Update descriptor set.
                vkUpdateDescriptorSets(Parent.GetVkDevice(),
                                       static_cast<uint32_t>(descriptorWrites.size()),
                                       descriptorWrites.data(), 0, nullptr);

                // Store descriptor set binding for current stream encoder position.
                vkCmdBindDescriptorSets(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        CurrPipeline->GetPipelineLayout(), set, 1, &descriptorSet,
                                        0, nullptr);

                // Add descriptor set to transient resource list.
                DeferredDeleters.push_back([descriptorSetLayout, descriptorSet]() -> void {
                    descriptorSetLayout->FreeDescriptorSet(descriptorSet);
                });
            }
        }
    }
}

}
