#include "DescriptorSetMetal.h"
#include "BufferMetal.h"
#include "DeviceMetal.h"
#include "ImageViewMetal.h"
#include "SamplerMetal.h"

namespace RHI
{

// --- CDescriptorSetLayoutMetal ---

CDescriptorSetLayoutMetal::CDescriptorSetLayoutMetal(
    CDeviceMetal& parent, const std::vector<CDescriptorSetLayoutBinding>& bindings)
    : Parent(parent)
    , Bindings(bindings)
{
}

CDescriptorSet::Ref CDescriptorSetLayoutMetal::CreateDescriptorSet()
{
    return std::make_shared<CDescriptorSetMetal>(Parent, *this);
}

// --- CPipelineLayoutMetal ---

CPipelineLayoutMetal::CPipelineLayoutMetal(const std::vector<CDescriptorSetLayout::Ref>& setLayouts)
{
    SetLayouts.reserve(setLayouts.size());
    for (uint32_t i = 0; i < setLayouts.size(); ++i)
    {
        auto metalLayout = std::static_pointer_cast<CDescriptorSetLayoutMetal>(setLayouts[i]);
        metalLayout->SetSetIndex(i);
        SetLayouts.push_back(metalLayout);
    }
}

// --- CDescriptorSetMetal ---

CDescriptorSetMetal::CDescriptorSetMetal(CDeviceMetal& parent, CDescriptorSetLayoutMetal& layout)
    : Parent(parent)
{
}

CMetalBoundResource& CDescriptorSetMetal::GetOrCreate(uint32_t binding)
{
    return BoundResources[binding];
}

void CDescriptorSetMetal::BindBuffer(CBuffer::Ref buffer, size_t offset, size_t range,
                                     uint32_t binding, uint32_t index)
{
    auto& res = GetOrCreate(binding);
    res.Type = CMetalBoundResource::Buffer;
    res.BoundBuffer = buffer;
    res.BufferOffset = offset;
    res.BufferRange = range;
}

void CDescriptorSetMetal::BindConstants(const void* data, size_t size, uint32_t binding,
                                        uint32_t index)
{
    auto& res = GetOrCreate(binding);
    res.Type = CMetalBoundResource::Constants;
    res.ConstantData.resize(size);
    memcpy(res.ConstantData.data(), data, size);
}

void CDescriptorSetMetal::BindImageView(CImageView::Ref imageView, uint32_t binding,
                                        uint32_t index)
{
    auto& res = GetOrCreate(binding);
    res.Type = CMetalBoundResource::ImageView;
    res.BoundImageView = imageView;
}

void CDescriptorSetMetal::BindSampler(CSampler::Ref sampler, uint32_t binding, uint32_t index)
{
    auto& res = GetOrCreate(binding);
    res.Type = CMetalBoundResource::Sampler;
    res.BoundSampler = sampler;
}

void CDescriptorSetMetal::BindBufferView(CBufferView::Ref bufferView, uint32_t binding,
                                         uint32_t index)
{
    // BufferView not directly supported in Metal; treat as no-op
}

void CDescriptorSetMetal::SetDynamicOffset(size_t offset, uint32_t binding, uint32_t index)
{
    auto it = BoundResources.find(binding);
    if (it != BoundResources.end() && it->second.Type == CMetalBoundResource::Buffer)
        it->second.BufferOffset = offset;
}

void CDescriptorSetMetal::ApplyToRenderEncoder(id encoder_,
                                               const CMSLBindingRemap& remap,
                                               uint32_t setIndex) const
{
    id<MTLRenderCommandEncoder> encoder = (id<MTLRenderCommandEncoder>)encoder_;
    for (auto& [binding, res] : BoundResources)
    {
        auto key = std::make_pair(setIndex, binding);

        if (res.Type == CMetalBoundResource::Buffer)
        {
            auto it = remap.BufferBindings.find(key);
            if (it != remap.BufferBindings.end())
            {
                auto* buf = static_cast<CBufferMetal*>(res.BoundBuffer.get());
                [encoder setVertexBuffer:buf->GetMTLBuffer()
                                  offset:res.BufferOffset
                                 atIndex:it->second];
                [encoder setFragmentBuffer:buf->GetMTLBuffer()
                                    offset:res.BufferOffset
                                   atIndex:it->second];
            }
        }
        else if (res.Type == CMetalBoundResource::Constants)
        {
            auto it = remap.BufferBindings.find(key);
            if (it != remap.BufferBindings.end())
            {
                [encoder setVertexBytes:res.ConstantData.data()
                                 length:res.ConstantData.size()
                                atIndex:it->second];
                [encoder setFragmentBytes:res.ConstantData.data()
                                   length:res.ConstantData.size()
                                  atIndex:it->second];
            }
        }
        else if (res.Type == CMetalBoundResource::ImageView)
        {
            auto it = remap.TextureBindings.find(key);
            if (it != remap.TextureBindings.end())
            {
                auto* view = static_cast<CImageViewMetal*>(res.BoundImageView.get());
                [encoder setVertexTexture:view->GetMTLTexture() atIndex:it->second];
                [encoder setFragmentTexture:view->GetMTLTexture() atIndex:it->second];
            }
        }
        else if (res.Type == CMetalBoundResource::Sampler)
        {
            auto it = remap.SamplerBindings.find(key);
            if (it != remap.SamplerBindings.end())
            {
                auto* sampler = static_cast<CSamplerMetal*>(res.BoundSampler.get());
                [encoder setVertexSamplerState:sampler->GetMTLSampler() atIndex:it->second];
                [encoder setFragmentSamplerState:sampler->GetMTLSampler() atIndex:it->second];
            }
        }
    }
}

void CDescriptorSetMetal::ApplyToComputeEncoder(id encoder_,
                                                const CMSLBindingRemap& remap,
                                                uint32_t setIndex) const
{
    id<MTLComputeCommandEncoder> encoder = (id<MTLComputeCommandEncoder>)encoder_;
    for (auto& [binding, res] : BoundResources)
    {
        auto key = std::make_pair(setIndex, binding);

        if (res.Type == CMetalBoundResource::Buffer)
        {
            auto it = remap.BufferBindings.find(key);
            if (it != remap.BufferBindings.end())
            {
                auto* buf = static_cast<CBufferMetal*>(res.BoundBuffer.get());
                [encoder setBuffer:buf->GetMTLBuffer() offset:res.BufferOffset atIndex:it->second];
            }
        }
        else if (res.Type == CMetalBoundResource::Constants)
        {
            auto it = remap.BufferBindings.find(key);
            if (it != remap.BufferBindings.end())
            {
                [encoder setBytes:res.ConstantData.data()
                           length:res.ConstantData.size()
                          atIndex:it->second];
            }
        }
        else if (res.Type == CMetalBoundResource::ImageView)
        {
            auto it = remap.TextureBindings.find(key);
            if (it != remap.TextureBindings.end())
            {
                auto* view = static_cast<CImageViewMetal*>(res.BoundImageView.get());
                [encoder setTexture:view->GetMTLTexture() atIndex:it->second];
            }
        }
        else if (res.Type == CMetalBoundResource::Sampler)
        {
            auto it = remap.SamplerBindings.find(key);
            if (it != remap.SamplerBindings.end())
            {
                auto* sampler = static_cast<CSamplerMetal*>(res.BoundSampler.get());
                [encoder setSamplerState:sampler->GetMTLSampler() atIndex:it->second];
            }
        }
    }
}

} /* namespace RHI */
