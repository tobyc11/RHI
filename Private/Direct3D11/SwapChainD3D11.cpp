#include "SwapChainD3D11.h"
#include "DeviceD3D11.h"
#include "ImageD3D11.h"
#include "RHIInstance.h"

namespace RHI
{

CSwapChainD3D11::CSwapChainD3D11(CDeviceD3D11& parent, IDXGISwapChain* inSwapChain)
    : Parent(parent)
    , SwapChain(inSwapChain)
{
}

CSwapChainD3D11::~CSwapChainD3D11() { SwapChain->Release(); }

void CSwapChainD3D11::Resize(uint32_t width, uint32_t height)
{
    DXGI_SWAP_CHAIN_DESC desc;
    SwapChain->GetDesc(&desc);
    SwapChain->ResizeBuffers(desc.BufferCount, width, height, desc.BufferDesc.Format, 0);
}

void CSwapChainD3D11::GetSize(uint32_t& width, uint32_t& height) const
{
    DXGI_SWAP_CHAIN_DESC desc;
    SwapChain->GetDesc(&desc);
    width = desc.BufferDesc.Width;
    height = desc.BufferDesc.Height;
}

CImage::Ref CSwapChainD3D11::GetImage()
{
    ComPtr<ID3D11Texture2D> pBackBuffer;
    SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                         reinterpret_cast<void**>(pBackBuffer.GetAddressOf()));
    return std::make_shared<CImageD3D11>(Parent, pBackBuffer);
}

void CSwapChainD3D11::AcquireNextImage() {}

void CSwapChainD3D11::Present(const CSwapChainPresentInfo& info) { SwapChain->Present(0, 0); }

} /* namespace RHI */
