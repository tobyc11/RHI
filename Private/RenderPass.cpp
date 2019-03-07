#include "RenderPass.h"

namespace Nome::RHI
{

CRenderTargetRef::CRenderTargetRef(CSwapChain* swapChain)
    : SwapChain(SwapChain)
{
    bIsSwapChain = true;
}

} /* namespace Nome::RHI */
