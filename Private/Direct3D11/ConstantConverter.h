#pragma once
#include "Format.h"
#include "Sampler.h"
#include "D3D11Platform.h"

namespace Nome::RHI
{

DXGI_FORMAT Convert(EFormat format);
D3D11_TEXTURE_ADDRESS_MODE Convert(ESamplerAddressMode mode);

} /* namespace Nome::RHI */
