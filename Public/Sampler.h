#pragma once
#include "PipelineStateDesc.h"
#include <Hash.h>
#include <cfloat>
#include <memory>

namespace RHI
{

enum class EFilter
{
    Nearest = 0,
    Linear = 1,
};

enum class ESamplerMipmapMode
{
    Nearest = 0,
    Linear = 1,
};

enum class ESamplerAddressMode
{
    Wrap = 1,
    Mirror = 2,
    Clamp = 3,
    Border = 4,
    MirrorOnce = 5,
};

struct CSamplerDesc
{
    EFilter MagFilter = EFilter::Linear;
    EFilter MinFilter = EFilter::Linear;
    ESamplerMipmapMode MipmapMode = ESamplerMipmapMode::Linear;
    ESamplerAddressMode AddressModeU = ESamplerAddressMode::Clamp;
    ESamplerAddressMode AddressModeV = ESamplerAddressMode::Clamp;
    ESamplerAddressMode AddressModeW = ESamplerAddressMode::Clamp;
    float MipLodBias = 0.0f;
    bool AnisotropyEnable = false;
    float MaxAnisotropy = 1.0f;
    bool CompareEnable = false;
    ECompareOp CompareOp = ECompareOp::Never;
    float MinLod = -FLT_MAX;
    float MaxLod = FLT_MAX;
    std::array<float, 4> BorderColor = { 1.0f, 1.0f, 1.0f, 1.0f };

    bool operator=(const CSamplerDesc& r) const
    {
        return MagFilter == r.MagFilter && MinFilter == r.MinFilter && MipmapMode == r.MipmapMode
            && AddressModeU == r.AddressModeU && AddressModeV == r.AddressModeV
            && AddressModeW == r.AddressModeW && MipLodBias == r.MipLodBias
            && AnisotropyEnable == r.AnisotropyEnable && MaxAnisotropy == r.MaxAnisotropy
            && CompareEnable == r.CompareEnable && CompareOp == r.CompareOp && MinLod == r.MinLod
            && MaxLod == r.MaxLod && BorderColor == r.BorderColor;
    }

    std::size_t hash(const CSamplerDesc& r)
    {
        std::size_t h = 0;
        tc::hash_combine(h, (std::underlying_type_t<EFilter>)(MagFilter));
        tc::hash_combine(h, (std::underlying_type_t<EFilter>)(MinFilter));
        tc::hash_combine(h, (std::underlying_type_t<ESamplerMipmapMode>)(MipmapMode));
        tc::hash_combine(h, (std::underlying_type_t<ESamplerAddressMode>)(AddressModeU));
        tc::hash_combine(h, (std::underlying_type_t<ESamplerAddressMode>)(AddressModeV));
        tc::hash_combine(h, (std::underlying_type_t<ESamplerAddressMode>)(AddressModeW));
        tc::hash_combine(h, MipLodBias);
        tc::hash_combine(h, AnisotropyEnable);
        tc::hash_combine(h, MaxAnisotropy);
        tc::hash_combine(h, CompareEnable);
        tc::hash_combine(h, (std::underlying_type_t<ECompareOp>)(CompareOp));
        tc::hash_combine(h, MinLod);
        tc::hash_combine(h, MaxLod);
        for (float f : BorderColor)
            tc::hash_combine(h, f);
        return h;
    }
};

class CSampler
{
public:
    typedef std::shared_ptr<CSampler> Ref;

    virtual ~CSampler() = default;
};

} /* namespace RHI */
