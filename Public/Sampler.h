#pragma once
#include "PipelineStateDesc.h"
#include <Hash.h>
#include <LangUtils.h>
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

    friend std::size_t hash_value(const CSamplerDesc& r)
    {
        std::size_t h = 0;
        tc::hash_combine(h, (std::underlying_type_t<EFilter>)(r.MagFilter));
        tc::hash_combine(h, (std::underlying_type_t<EFilter>)(r.MinFilter));
        tc::hash_combine(h, (std::underlying_type_t<ESamplerMipmapMode>)(r.MipmapMode));
        tc::hash_combine(h, (std::underlying_type_t<ESamplerAddressMode>)(r.AddressModeU));
        tc::hash_combine(h, (std::underlying_type_t<ESamplerAddressMode>)(r.AddressModeV));
        tc::hash_combine(h, (std::underlying_type_t<ESamplerAddressMode>)(r.AddressModeW));
        tc::hash_combine(h, r.MipLodBias);
        tc::hash_combine(h, r.AnisotropyEnable);
        tc::hash_combine(h, r.MaxAnisotropy);
        tc::hash_combine(h, r.CompareEnable);
        tc::hash_combine(h, (std::underlying_type_t<ECompareOp>)(r.CompareOp));
        tc::hash_combine(h, r.MinLod);
        tc::hash_combine(h, r.MaxLod);
        for (float f : r.BorderColor)
            tc::hash_combine(h, f);
        return h;
    }
};

class CSampler : public tc::FNonCopyable
{
public:
    typedef std::shared_ptr<CSampler> Ref;

    virtual ~CSampler() = default;
};

} /* namespace RHI */
