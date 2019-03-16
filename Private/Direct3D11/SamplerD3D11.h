#pragma once
#include "Sampler.h"
#include "D3D11Platform.h"

namespace RHI
{

class CSamplerD3D11 : public CSampler
{
public:
    CSamplerD3D11(ID3D11SamplerState* state);

    ID3D11SamplerState* GetSamplerState() const { return SamplerState.Get(); }

private:
    ComPtr<ID3D11SamplerState> SamplerState;
};

} /* namespace RHI */
