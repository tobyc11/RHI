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

class CSampler : public tc::CVirtualLightRefBase
{
public:
    //We've got a virtual destructor from tc::CVirtualLightRefBase
};

} /* namespace RHI */
