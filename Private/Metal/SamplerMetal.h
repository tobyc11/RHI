#pragma once
#include "MtlCommon.h"
#include "Sampler.h"

namespace RHI
{

class CSamplerMetal : public CSampler
{
public:
    typedef std::shared_ptr<CSamplerMetal> Ref;

    CSamplerMetal(CDeviceMetal& parent, const CSamplerDesc& desc);
    ~CSamplerMetal() override;

    id GetMTLSampler() const { return Sampler; }

private:
    id Sampler;
};

} /* namespace RHI */
