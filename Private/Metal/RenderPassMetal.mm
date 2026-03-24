#include "RenderPassMetal.h"
#include "ImageViewMetal.h"
#include "MtlHelpers.h"

namespace RHI
{

CRenderPassMetal::CRenderPassMetal(CDeviceMetal& parent, const CRenderPassDesc& desc)
    : Desc(desc)
{
}

void CRenderPassMetal::SetSize(uint32_t width, uint32_t height)
{
    Desc.Width = width;
    Desc.Height = height;
}

MTLRenderPassDescriptor* CRenderPassMetal::CreateMTLRenderPassDescriptor(
    uint32_t subpass, const std::vector<CClearValue>& clearValues) const
{
    MTLRenderPassDescriptor* rpd = [MTLRenderPassDescriptor renderPassDescriptor];

    if (subpass >= Desc.Subpasses.size())
        return rpd;

    const auto& sp = Desc.Subpasses[subpass];

    for (uint32_t i = 0; i < sp.ColorAttachments.size(); ++i)
    {
        uint32_t attIdx = sp.ColorAttachments[i];
        if (attIdx >= Desc.Attachments.size())
            continue;

        const auto& att = Desc.Attachments[attIdx];
        auto* view = static_cast<CImageViewMetal*>(att.ImageView.get());

        rpd.colorAttachments[i].texture = view->GetMTLTexture();
        rpd.colorAttachments[i].loadAction = MtlCast(att.LoadOp);
        rpd.colorAttachments[i].storeAction = MtlCast(att.StoreOp);

        if (att.LoadOp == EAttachmentLoadOp::Clear && attIdx < clearValues.size())
        {
            const auto& cv = clearValues[attIdx];
            rpd.colorAttachments[i].clearColor =
                MTLClearColorMake(cv.ColorFloat32[0], cv.ColorFloat32[1], cv.ColorFloat32[2],
                                  cv.ColorFloat32[3]);
        }
    }

    if (sp.DepthStencilAttachment != CSubpassDesc::None)
    {
        uint32_t dsIdx = sp.DepthStencilAttachment;
        if (dsIdx < Desc.Attachments.size())
        {
            const auto& att = Desc.Attachments[dsIdx];
            auto* view = static_cast<CImageViewMetal*>(att.ImageView.get());
            id<MTLTexture> tex = view->GetMTLTexture();
            MTLPixelFormat pf = tex.pixelFormat;

            bool hasDepth = (pf == MTLPixelFormatDepth16Unorm ||
                             pf == MTLPixelFormatDepth32Float ||
                             pf == MTLPixelFormatDepth24Unorm_Stencil8 ||
                             pf == MTLPixelFormatDepth32Float_Stencil8);
            bool hasStencil = (pf == MTLPixelFormatStencil8 ||
                               pf == MTLPixelFormatDepth24Unorm_Stencil8 ||
                               pf == MTLPixelFormatDepth32Float_Stencil8);

            if (hasDepth)
            {
                rpd.depthAttachment.texture = tex;
                rpd.depthAttachment.loadAction = MtlCast(att.LoadOp);
                rpd.depthAttachment.storeAction = MtlCast(att.StoreOp);
                if (att.LoadOp == EAttachmentLoadOp::Clear && dsIdx < clearValues.size())
                    rpd.depthAttachment.clearDepth = clearValues[dsIdx].DepthStencilValue.Depth;
            }

            if (hasStencil)
            {
                rpd.stencilAttachment.texture = tex;
                rpd.stencilAttachment.loadAction = MtlCast(att.StencilLoadOp);
                rpd.stencilAttachment.storeAction = MtlCast(att.StencilStoreOp);
                if (att.StencilLoadOp == EAttachmentLoadOp::Clear && dsIdx < clearValues.size())
                    rpd.stencilAttachment.clearStencil =
                        clearValues[dsIdx].DepthStencilValue.Stencil;
            }
        }
    }

    rpd.renderTargetWidth = Desc.Width;
    rpd.renderTargetHeight = Desc.Height;

    return rpd;
}

} /* namespace RHI */
