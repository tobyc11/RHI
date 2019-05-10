#include "DescriptorSetVk.h"
#include "DeviceVk.h"
#include "ImageViewVk.h"
#include "SamplerVk.h"
#include <cstring>
#include <mutex>

namespace RHI
{

RHI::CDescriptorSetVk::CDescriptorSetVk(CDescriptorSetLayoutVk::Ref layout)
    : Layout(layout)
{
}

RHI::CDescriptorSetVk::~CDescriptorSetVk() {}

void RHI::CDescriptorSetVk::BindBuffer(CBuffer::Ref buffer, size_t offset, size_t range,
                                       uint32_t binding, uint32_t index)
{
    auto handle = std::static_pointer_cast<CBufferVk>(buffer)->GetHandle();
    ResourceBindings.BindBuffer(handle, offset, range, 0, binding, index);
}

void CDescriptorSetVk::BindConstants(const void* data, size_t size, uint32_t binding,
                                     uint32_t index)
{
    auto* bufferImpl = Layout->GetDevice().GetHugeConstantBuffer();
    size_t offset;
    size_t minAlignment = Layout->GetDevice().GetVkLimits().minUniformBufferOffsetAlignment;
    void* bufferData = bufferImpl->Allocate(size, minAlignment, offset);
    memcpy(bufferData, data, size);
    ResourceBindings.BindBuffer(bufferImpl->GetHandle(), offset, size, 0, binding, index);
}

void RHI::CDescriptorSetVk::BindImageView(CImageView::Ref imageView, uint32_t binding,
                                          uint32_t index)
{
    auto impl = std::static_pointer_cast<CImageViewVk>(imageView);

    VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    if (Any(impl->GetImage()->GetUsageFlags(), EImageUsageFlags::Storage))
        layout = VK_IMAGE_LAYOUT_GENERAL;
    if (GetImageAspectFlags(impl->GetFormat()) != VK_IMAGE_ASPECT_COLOR_BIT)
        layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    ResourceBindings.BindImageView(impl.get(), layout, VK_NULL_HANDLE, 0, binding, index);
}

void RHI::CDescriptorSetVk::BindSampler(CSampler::Ref sampler, uint32_t binding, uint32_t index)
{
    auto impl = std::static_pointer_cast<CSamplerVk>(sampler);
    ResourceBindings.BindSampler(impl->Sampler, 0, binding, index);
}

void RHI::CDescriptorSetVk::BindBufferView(CBufferView::Ref bufferView, uint32_t binding,
                                           uint32_t index)
{
    throw "unimplemented";
}

void CDescriptorSetVk::SetDynamicOffset(size_t offset, uint32_t binding, uint32_t index)
{
    throw "unimplemented";
}

VkDescriptorSet RHI::CDescriptorSetVk::GetHandle() { return Handle; }

void CDescriptorSetVk::DiscardAndRecreate()
{
    // Discard the old handle
    if (Handle)
    {
        auto l = Layout;
        auto h = Handle;
        Layout->GetDevice().AddPostFrameCleanup(
            [l, h](CDeviceVk& p) { l->GetDescriptorPool()->FreeDescriptorSet(h); });
    }

    const auto& poolPtr = Layout->GetDescriptorPool();
    Handle = poolPtr->AllocateDescriptorSet();
}

void CDescriptorSetVk::WriteUpdates(CAccessTracker& tracker, VkCommandBuffer cmdBuffer)
{
    if (!ResourceBindings.IsDirty())
        return;

    ResourceBindings.ClearDirtyBit();

    if (bIsUsed || !Handle)
    {
        DiscardAndRecreate();
        bIsUsed = false;
    }

    // Since we only have one descriptor set, the first SetBindings is it
    auto& setBindings = ResourceBindings.GetSetBindings().begin()->second;
    setBindings.bDirty = false;

    std::vector<VkWriteDescriptorSet> writes;
    std::vector<VkDescriptorImageInfo> imageInfos;
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    std::vector<VkBufferView> bufferViews;
    for (const auto& bindingIter : setBindings.Bindings)
    {
        uint32_t binding = bindingIter.first;
        for (const auto& arrayIter : bindingIter.second)
        {
            uint32_t index = arrayIter.first;

            writes.push_back({ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET });
            auto& w = writes.back();
            w.dstSet = Handle;
            w.dstBinding = binding;
            w.dstArrayElement = index;
            w.descriptorCount = 1;
            w.descriptorType = Layout->GetDescriptorType(binding);

            if (arrayIter.second.ImageView)
            {
                VkDescriptorImageInfo info {};
                info.imageView = arrayIter.second.ImageView->GetVkImageView();
                info.imageLayout = arrayIter.second.ImageLayout;

                tracker.TransitionImage(cmdBuffer, arrayIter.second.ImageView->GetImage().get(),
                                        arrayIter.second.ImageView->GetResourceRange(),
                                        VK_ACCESS_SHADER_READ_BIT,
                                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, info.imageLayout);

                imageInfos.push_back(info);
                w.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(imageInfos.size());
            }
            else if (arrayIter.second.BufferHandle)
            {
                VkDescriptorBufferInfo info {};
                info.buffer = arrayIter.second.BufferHandle;
                info.offset = arrayIter.second.Offset;
                info.range = arrayIter.second.Range;

                bufferInfos.push_back(info);
                w.pBufferInfo = reinterpret_cast<const VkDescriptorBufferInfo*>(bufferInfos.size());
            }
            else if (arrayIter.second.SamplerHandle)
            {
                VkDescriptorImageInfo info {};
                info.sampler = arrayIter.second.SamplerHandle;

                imageInfos.push_back(info);
                w.pImageInfo = reinterpret_cast<const VkDescriptorImageInfo*>(imageInfos.size());
            }
        }
    }

    for (auto& w : writes)
    {
        if (w.pImageInfo)
            w.pImageInfo = &imageInfos[reinterpret_cast<size_t>(w.pImageInfo) - 1];
        if (w.pBufferInfo)
            w.pBufferInfo = &bufferInfos[reinterpret_cast<size_t>(w.pBufferInfo) - 1];
        if (w.pTexelBufferView)
            w.pTexelBufferView = &bufferViews[reinterpret_cast<size_t>(w.pTexelBufferView) - 1];
    }

    vkUpdateDescriptorSets(Layout->GetDevice().GetVkDevice(), static_cast<uint32_t>(writes.size()),
                           writes.data(), 0, nullptr);
}

}
