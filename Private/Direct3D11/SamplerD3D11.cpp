#include "SamplerD3D11.h"

namespace RHI
{

CSamplerD3D11::CSamplerD3D11(ID3D11SamplerState* state)
    : SamplerState(state)
{
}

} /* namespace RHI */
