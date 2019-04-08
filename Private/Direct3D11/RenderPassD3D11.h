#pragma once
#include "D3D11Platform.h"
#include "ImageD3D11.h"
#include "RenderContext.h"
#include "RenderPass.h"
#include <vector>

namespace RHI
{

struct CSubpassInfo
{
    std::vector<ComPtr<ID3D11RenderTargetView>> RTVs;
    ComPtr<ID3D11DepthStencilView> DSV;
};

class CRenderPassD3D11 : public CRenderPass
{
public:
    CRenderPassD3D11(CDeviceD3D11& p, const CRenderPassDesc& desc);

    void Bind(ID3D11DeviceContext* ctx, size_t subpass,
              const std::vector<CClearValue>& clearValues);

private:
    CDeviceD3D11& Parent;
    std::vector<CImageViewD3D11::Ref> Attachments;
    std::vector<uint32_t> AttachmentInfo;
    std::vector<CSubpassInfo> Subpasses;
    float Width, Height;
};

}