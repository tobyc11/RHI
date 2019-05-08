#include "DescriptorSetVk.h"
#include "SamplerVk.h"
#include "DeviceVk.h"
#include <mutex>

namespace RHI
{

RHI::CDescriptorSetVk::CDescriptorSetVk(CDescriptorSetLayoutVk::Ref layout) : Layout(layout)
{
    DiscardAndRecreate();
}

RHI::CDescriptorSetVk::~CDescriptorSetVk()
{
}

void
RHI::CDescriptorSetVk::BindBuffer(CBuffer::Ref buffer, size_t offset, size_t range, uint32_t binding, uint32_t index)
{
    auto handle = std::static_pointer_cast<CBufferVk>(buffer)->Buffer;
    ResourceBindings.BindBuffer(handle, offset, range, 0, binding, index);
}

void RHI::CDescriptorSetVk::BindImageView(CImageView::Ref imageView, uint32_t binding, uint32_t index)
{
    auto impl = std::static_pointer_cast<CImageViewVk>(imageView);
    ResourceBindings.BindImageView(impl.get(), VK_NULL_HANDLE, 0, binding, index);
}

void RHI::CDescriptorSetVk::BindSampler(CSampler::Ref sampler, uint32_t binding, uint32_t index)
{
    auto impl = std::static_pointer_cast<CSamplerVk>(sampler);
    ResourceBindings.BindSampler(impl->Sampler, 0, binding, index);
}

void RHI::CDescriptorSetVk::BindBufferView(CBufferView::Ref bufferView, uint32_t binding, uint32_t index)
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

        Layout->GetDevice().AddPostFrameCleanup([=, &poolPtr](CDeviceVk& p){
            poolPtr->FreeDescriptorSet(Handle);
        });
    }

    Handle = poolPtr->AllocateDescriptorSet();
}

}
