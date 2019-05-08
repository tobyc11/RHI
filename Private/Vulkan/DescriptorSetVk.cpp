#include "DescriptorSetVk.h"
#include "DeviceVk.h"
#include "ImageViewVk.h"
#include "SamplerVk.h"
#include <mutex>

namespace RHI
{

RHI::CDescriptorSetVk::CDescriptorSetVk(CDescriptorSetLayoutVk::Ref layout)
    : Layout(layout)
{
    DiscardAndRecreate();
}

RHI::CDescriptorSetVk::~CDescriptorSetVk() {}

void RHI::CDescriptorSetVk::BindBuffer(CBuffer::Ref buffer, size_t offset, size_t range,
                                       uint32_t binding, uint32_t index)
{
    auto handle = std::static_pointer_cast<CBufferVk>(buffer)->Buffer;
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
    ResourceBindings.BindImageView(impl.get(), VK_NULL_HANDLE, 0, binding, index);
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

VkDescriptorSet RHI::CDescriptorSetVk::GetHandle(bool isUsedByCommand)
{
    if (isUsedByCommand)
        bIsUsed = true;
    return Handle;
}

void CDescriptorSetVk::DiscardAndRecreate()
{
    const auto& poolPtr = Layout->GetDescriptorPool();

    // Discard the old handle
    if (Handle)
    {
        // Dont need to do anything if the old handle is not used
        if (!bIsUsed)
            return;

        Layout->GetDevice().AddPostFrameCleanup(
            [=, &poolPtr](CDeviceVk& p) { poolPtr->FreeDescriptorSet(Handle); });
    }

    Handle = poolPtr->AllocateDescriptorSet();
}

void CDescriptorSetVk::WriteUpdates()
{
    if (!ResourceBindings.IsDirty())
        return;

    ResourceBindings.ClearDirtyBit();

    // Since we only have one descriptor set, the first SetBindings is it
    auto& setBindings = ResourceBindings.GetSetBindings().begin()->second;
    setBindings.bDirty = false;

    std::vector<VkWriteDescriptorSet> writes;
    std::vector<VkDescriptorImageInfo> imageInfos;
    imageInfos.reserve(128);
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    bufferInfos.reserve(128);
    std::vector<VkBufferView> bufferViews;
    bufferViews.reserve(128);
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
                info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                if (GetImageAspectFlags(arrayIter.second.ImageView->GetFormat())
                    != VK_IMAGE_ASPECT_COLOR_BIT)
                    info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

                imageInfos.push_back(info);
                w.pImageInfo = &imageInfos.back();
            }
            else if (arrayIter.second.BufferHandle)
            {
                VkDescriptorBufferInfo info {};
                info.buffer = arrayIter.second.BufferHandle;
                info.offset = arrayIter.second.Offset;
                info.range = arrayIter.second.Range;

                bufferInfos.push_back(info);
                w.pBufferInfo = &bufferInfos.back();
            }
            else if (arrayIter.second.SamplerHandle)
            {
                VkDescriptorImageInfo info {};
                info.sampler = arrayIter.second.SamplerHandle;

                imageInfos.push_back(info);
                w.pImageInfo = &imageInfos.back();
            }
        }
    }

    vkUpdateDescriptorSets(Layout->GetDevice().GetVkDevice(), writes.size(), writes.data(), 0,
                           nullptr);
}

}
