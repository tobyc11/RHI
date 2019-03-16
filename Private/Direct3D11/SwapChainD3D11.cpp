#include "SwapChainD3D11.h"
#include "DeviceD3D11.h"
#include "ImageD3D11.h"
#include "RHIInstance.h"

namespace RHI
{

CSwapChainD3D11::CSwapChainD3D11(IDXGISwapChain* inSwapChain)
    : SwapChain(inSwapChain)
{
}

CSwapChainD3D11::~CSwapChainD3D11()
{
    SwapChain->Release();
}

sp<CImage> CSwapChainD3D11::GetImage(uint32_t index)
{
    ID3D11Texture2D* pBackBuffer;
    SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
    auto* devImpl = static_cast<CDeviceD3D11*>(CInstance::Get().GetCurrDevice());
    return new CImageD3D11(devImpl, pBackBuffer, 2);
}

void CSwapChainD3D11::Resize(int width, int height)
{
    DXGI_SWAP_CHAIN_DESC desc;
    SwapChain->GetDesc(&desc);
    SwapChain->ResizeBuffers(desc.BufferCount, width, height, desc.BufferDesc.Format, 0);
}

void CSwapChainD3D11::GetSize(int& width, int& height) const
{
    DXGI_SWAP_CHAIN_DESC desc;
    SwapChain->GetDesc(&desc);
    width = desc.BufferDesc.Width;
    height = desc.BufferDesc.Height;
}

void CSwapChainD3D11::Present()
{
    SwapChain->Present(0, 0);
}

} /* namespace RHI */
