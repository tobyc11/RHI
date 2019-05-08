#pragma once
#include "RHIChooseImpl.h"
#include "ShaderModule.h"
#include <memory>

namespace RHI
{

enum class EDescriptorType
{
    Sampler,
    Image,
    StorageImage,
    UniformTexelBuffer,
    StorageTexelBuffer,
    UniformBuffer,
    StorageBuffer,
    UniformBufferDynamic,
    StorageBufferDynamic,
    InputAttachment
};

struct CDescriptorSetLayoutBinding
{
    uint32_t Binding;
    EDescriptorType Type;
    uint32_t Count;
    EShaderStageFlags StageFlags;
};

class CDescriptorSet
{
public:
    typedef std::shared_ptr<CDescriptorSet> Ref;

    virtual ~CDescriptorSet() = default;

    virtual void BindBuffer(CBuffer::Ref buffer, size_t offset, size_t range, uint32_t binding, uint32_t index) = 0;
    virtual void BindImageView(CImageView::Ref imageView, uint32_t binding, uint32_t index) = 0;
    virtual void BindSampler(CSampler::Ref sampler, uint32_t binding, uint32_t index) = 0;
    virtual void BindBufferView(CBufferView::Ref bufferView, uint32_t binding, uint32_t index) = 0;
    virtual void SetDynamicOffset(size_t offset, uint32_t binding, uint32_t index) = 0;
};

class CDescriptorSetLayout : public std::enable_shared_from_this<CDescriptorSetLayout>
{
public:
    typedef std::shared_ptr<CDescriptorSetLayout> Ref;

    virtual ~CDescriptorSetLayout() = default;

    // Allocate and create a descriptor set from this layout
    virtual CDescriptorSet::Ref CreateDescriptorSet() = 0;
};

class CPipelineLayout
{
public:
    typedef std::shared_ptr<CPipelineLayout> Ref;

    virtual ~CPipelineLayout() = default;
};

}
