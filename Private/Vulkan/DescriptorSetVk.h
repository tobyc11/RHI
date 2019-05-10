#pragma once
#include "AccessTracker.h"
#include "DescriptorSetLayoutVk.h"
#include "ResourceBindingsVk.h"

namespace RHI
{

class CDescriptorSetVk : public CDescriptorSet
{
public:
    typedef std::shared_ptr<CDescriptorSetVk> Ref;

    explicit CDescriptorSetVk(CDescriptorSetLayoutVk::Ref layout);
    ~CDescriptorSetVk() override;

    void BindBuffer(CBuffer::Ref buffer, size_t offset, size_t range, uint32_t binding,
                    uint32_t index) override;
    void BindConstants(const void* data, size_t size, uint32_t binding, uint32_t index) override;
    void BindImageView(CImageView::Ref imageView, uint32_t binding, uint32_t index) override;
    void BindSampler(CSampler::Ref sampler, uint32_t binding, uint32_t index) override;
    void BindBufferView(CBufferView::Ref bufferView, uint32_t binding, uint32_t index) override;
    void SetDynamicOffset(size_t offset, uint32_t binding, uint32_t index) override;

    // Internal API
    VkDescriptorSet GetHandle();
    bool IsContentDirty() const { return ResourceBindings.IsDirty(); }
    void DiscardAndRecreate(); // Similar to the DX11 MapDiscard semantics
    void WriteUpdates(CAccessTracker& tracker, VkCommandBuffer cmdBuffer);
    void SetUsed() { bIsUsed = true; }

private:
    // Holds the layout alive
    CDescriptorSetLayoutVk::Ref Layout;

    // NOTE: lazy create and update
    CResourceBindings ResourceBindings;
    VkDescriptorSet Handle = VK_NULL_HANDLE;

    // If used, we can't freely update this anymore
    bool bIsUsed = false;
};

}
