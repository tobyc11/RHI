#include "SwapChainD3D11.h"
#include "DeviceD3D11.h"
#include "ImageD3D11.h"
#include "RHIInstance.h"

namespace RHI
{

CSwapChainD3D11::CSwapChainD3D11(CDeviceD3D11& parent, ComPtr<IDXGISwapChain> inSwapChain)
    : Parent(parent)
    , SwapChain(inSwapChain)
{
}

void CSwapChainD3D11::Resize(uint32_t width, uint32_t height)
{
    DXGI_SWAP_CHAIN_DESC desc;
    SwapChain->GetDesc(&desc);
    if (width == UINT32_MAX && height == UINT32_MAX)
    {
        RECT rect;
        GetClientRect(desc.OutputWindow, &rect);
        UINT wndWidth = rect.right;
        UINT wndHeight = rect.bottom;
        SwapChain->ResizeBuffers(desc.BufferCount, wndWidth, wndHeight, desc.BufferDesc.Format, 0);
    }
    else
        SwapChain->ResizeBuffers(desc.BufferCount, width, height, desc.BufferDesc.Format, 0);
    bNeedResize = false;
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

bool CSwapChainD3D11::AcquireNextImage() { return !bNeedResize; }

void CSwapChainD3D11::Present(const CSwapChainPresentInfo& info)
{
    HRESULT hr = SwapChain->Present(0, 0);
    if (hr == DXGI_STATUS_OCCLUDED)
    {
        // TODO: wait a bit
    }

    DXGI_SWAP_CHAIN_DESC desc;
    SwapChain->GetDesc(&desc);
    RECT rect;
    GetClientRect(desc.OutputWindow, &rect);
    UINT wndWidth = rect.right;
    UINT wndHeight = rect.bottom;
    if (wndWidth != desc.BufferDesc.Width || wndHeight != desc.BufferDesc.Height)
        bNeedResize = true;
}

} /* namespace RHI */
