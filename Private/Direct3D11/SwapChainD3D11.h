#pragma once
#include "D3D11Platform.h"
#include "SwapChain.h"

namespace RHI
{

class CSwapChainD3D11 : public CSwapChain
{
public:
    // Only for internal use
    CSwapChainD3D11(CDeviceD3D11& parent, IDXGISwapChain* inSwapChain);
    ~CSwapChainD3D11();

    // Inherited via CSwapChain
    void Resize(uint32_t width, uint32_t height) override;
    void GetSize(uint32_t& width, uint32_t& height) const override;

    CImage::Ref GetImage() override;

    void AcquireNextImage() override;
    void Present(const CSwapChainPresentInfo& info) override;

private:
    CDeviceD3D11& Parent;
    IDXGISwapChain* SwapChain;
};

} /* namespace RHI */
