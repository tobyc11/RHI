#pragma once
#include "Sampler.h"
#include "VkCommon.h"

namespace RHI
{

class CSamplerVk : public CSampler
{
public:
    CSamplerVk(CDeviceVk& p, const CSamplerDesc& desc);
    ~CSamplerVk();

	VkSampler Sampler;

private:
    CDeviceVk& Parent;
};

} /* namespace RHI */
