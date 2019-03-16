#pragma once
#include "SwapChain.h"
#include "D3D11Platform.h"

namespace RHI
{

class CSwapChainD3D11 : public CSwapChain
{
public:
    //Only for internal use
    CSwapChainD3D11(IDXGISwapChain* inSwapChain);
    ~CSwapChainD3D11();

    // Inherited via CSwapChain
    sp<CImage> GetImage(uint32_t index) override;
    void Resize(int width, int height) override;
    void GetSize(int& width, int& height) const override;
    void Present() override;

private:
    IDXGISwapChain* SwapChain;
};

} /* namespace RHI */
