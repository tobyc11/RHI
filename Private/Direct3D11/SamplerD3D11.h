#pragma once
#include "Sampler.h"
#include "D3D11Platform.h"

namespace Nome::RHI
{

class CSamplerD3D11 : public CSampler
{
public:
    CSamplerD3D11(ID3D11SamplerState* state);

private:
    ComPtr<ID3D11SamplerState> SamplerState;
};

} /* namespace Nome::RHI */
