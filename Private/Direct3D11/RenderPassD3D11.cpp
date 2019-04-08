#include "RenderPassD3D11.h"

namespace RHI
{

#define ATCH_COLOR_BIT 1
#define ATCH_DEPTH_BIT 2

RHI::CRenderPassD3D11::CRenderPassD3D11(CDeviceD3D11& p, const CRenderPassDesc& desc)
    : Parent(p)
{
    for (const auto& a : desc.Attachments)
        Attachments.push_back(std::static_pointer_cast<CImageViewD3D11>(a.ImageView));
    AttachmentInfo.resize(Attachments.size());

    for (const auto& sp : desc.Subpasses)
    {
        CSubpassInfo info;
        for (uint32_t attachmentIndex : sp.ColorAttachments)
        {
            info.RTVs.push_back(Attachments[attachmentIndex]->GetRenderTargetView());
            AttachmentInfo[attachmentIndex] |= ATCH_COLOR_BIT;
        }
        if (sp.DepthStencilAttachment != sp.None)
        {
            info.DSV = Attachments[sp.DepthStencilAttachment]->GetDepthStencilView();
            AttachmentInfo[sp.DepthStencilAttachment] |= ATCH_DEPTH_BIT;
        }
        Subpasses.push_back(info);
    }

    for (uint32_t flags : AttachmentInfo)
        if ((flags & (ATCH_COLOR_BIT | ATCH_DEPTH_BIT)) == (ATCH_COLOR_BIT | ATCH_DEPTH_BIT))
            throw CRHIRuntimeError("An attachment used as both color and depth???");

    Width = static_cast<float>(desc.Width);
    Height = static_cast<float>(desc.Height);
}

void RHI::CRenderPassD3D11::Bind(ID3D11DeviceContext* ctx, size_t subpass,
                                 const std::vector<CClearValue>& clearValues)
{
    ctx->OMSetRenderTargets(
        static_cast<uint32_t>(Subpasses[subpass].RTVs.size()),
        reinterpret_cast<ID3D11RenderTargetView* const*>(Subpasses[subpass].RTVs.data()),
        Subpasses[subpass].DSV.Get());

    if (subpass == 0)
    {
        // Clear attachments if beginning a render pass
        for (size_t i = 0; i < clearValues.size(); i++)
        {
            if (AttachmentInfo[i] & ATCH_COLOR_BIT)
                ctx->ClearRenderTargetView(Attachments[i]->GetRenderTargetView().Get(),
                                           clearValues[i].ColorFloat32);
            else if (AttachmentInfo[i] & ATCH_DEPTH_BIT)
                ctx->ClearDepthStencilView(Attachments[i]->GetDepthStencilView().Get(),
                                           D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                           clearValues[i].Depth,
                                           static_cast<UINT8>(clearValues[i].Stencil));
        }
    }

    D3D11_VIEWPORT vp;
    vp.TopLeftX = vp.TopLeftY = 0;
    vp.Width = Width;
    vp.Height = Height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    ctx->RSSetViewports(1, &vp);
}

}
