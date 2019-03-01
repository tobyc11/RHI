#pragma once
#include "PipelineStateDesc.h"
#include <RefBase.h>

namespace Nome::RHI
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
    Wrap = 0,
    Mirror = 1,
    Clamp = 2,
    Border = 3,
    MirrorOnce = 4,
};

struct CSamplerDesc
{
    EFilter MagFilter;
    EFilter MinFilter;
    ESamplerMipmapMode MipmapMode;
    ESamplerAddressMode AddressModeU;
    ESamplerAddressMode AddressModeV;
    ESamplerAddressMode AddressModeW;
    float MipLodBias;
    bool AnisotropyEnable;
    float MaxAnisotropy;
    bool CompareEnable;
    ECompareOp CompareOp;
    float MinLod;
    float MaxLod;
    float BorderColor[4];
};

class CSampler : public tc::TLightRefBase<CSampler>
{
public:
};

} /* namespace Nome::RHI */
