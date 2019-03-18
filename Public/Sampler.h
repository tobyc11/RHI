#pragma once
#include "PipelineStateDesc.h"
#include <RefBase.h>

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
};

class CSampler : public tc::CVirtualLightRefBase
{
public:
    //We've got a virtual destructor from tc::CVirtualLightRefBase
};

} /* namespace RHI */
