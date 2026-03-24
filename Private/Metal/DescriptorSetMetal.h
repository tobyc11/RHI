#pragma once
#include "DescriptorSet.h"
#include "MtlCommon.h"
#include "SPIRVToMSL.h"

namespace RHI
{

class CDescriptorSetLayoutMetal : public CDescriptorSetLayout
{
public:
    typedef std::shared_ptr<CDescriptorSetLayoutMetal> Ref;

    CDescriptorSetLayoutMetal(CDeviceMetal& parent,
                              const std::vector<CDescriptorSetLayoutBinding>& bindings);

    CDescriptorSet::Ref CreateDescriptorSet() override;

    const std::vector<CDescriptorSetLayoutBinding>& GetBindings() const { return Bindings; }
    uint32_t GetSetIndex() const { return SetIndex; }
    void SetSetIndex(uint32_t idx) { SetIndex = idx; }

private:
    CDeviceMetal& Parent;
    std::vector<CDescriptorSetLayoutBinding> Bindings;
    uint32_t SetIndex = 0;
};

class CPipelineLayoutMetal : public CPipelineLayout
{
public:
    typedef std::shared_ptr<CPipelineLayoutMetal> Ref;

    CPipelineLayoutMetal(const std::vector<CDescriptorSetLayout::Ref>& setLayouts);

    const std::vector<CDescriptorSetLayoutMetal::Ref>& GetSetLayouts() const { return SetLayouts; }

private:
    std::vector<CDescriptorSetLayoutMetal::Ref> SetLayouts;
};

struct CMetalBoundResource
{
    enum EType
    {
        None,
        Buffer,
        ImageView,
        Sampler,
        Constants
    };

    EType Type = None;
    CBuffer::Ref BoundBuffer;
    size_t BufferOffset = 0;
    size_t BufferRange = 0;
    CImageView::Ref BoundImageView;
    CSampler::Ref BoundSampler;
    std::vector<uint8_t> ConstantData;
};

class CDescriptorSetMetal : public CDescriptorSet
{
public:
    typedef std::shared_ptr<CDescriptorSetMetal> Ref;

    CDescriptorSetMetal(CDeviceMetal& parent, CDescriptorSetLayoutMetal& layout);

    void BindBuffer(CBuffer::Ref buffer, size_t offset, size_t range, uint32_t binding,
                    uint32_t index) override;
    void BindConstants(const void* data, size_t size, uint32_t binding, uint32_t index) override;
    void BindImageView(CImageView::Ref imageView, uint32_t binding, uint32_t index) override;
    void BindSampler(CSampler::Ref sampler, uint32_t binding, uint32_t index) override;
    void BindBufferView(CBufferView::Ref bufferView, uint32_t binding, uint32_t index) override;
    void SetDynamicOffset(size_t offset, uint32_t binding, uint32_t index) override;

    void ApplyToRenderEncoder(id encoder, const CMSLBindingRemap& remap,
                              uint32_t setIndex) const;
    void ApplyToComputeEncoder(id encoder, const CMSLBindingRemap& remap,
                               uint32_t setIndex) const;

private:
    CMetalBoundResource& GetOrCreate(uint32_t binding);

    CDeviceMetal& Parent;
    std::map<uint32_t, CMetalBoundResource> BoundResources;
};

} /* namespace RHI */
