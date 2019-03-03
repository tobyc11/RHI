#include "SwapChainD3D11.h"

namespace Nome::RHI
{

CSwapChainD3D11::CSwapChainD3D11(IDXGISwapChain* inSwapChain)
    : SwapChain(inSwapChain)
{
}

CSwapChainD3D11::~CSwapChainD3D11()
{
    delete SwapChain;
}

void CSwapChainD3D11::Resize(int width, int height)
{
}

} /* namespace Nome::RHI */
