#include "RenderPassD3D11.h"
#include "RenderGraph.h"
#include "ImageD3D11.h"
#include "RHIInstance.h"

namespace RHI
{

///////////////////////////////////////////////////////////////////////////////
// Draw pass
///////////////////////////////////////////////////////////////////////////////

CDrawPass::Impl::Impl(CDrawPass& owner) : Owner(owner)
{
    DeviceImpl = static_cast<CDeviceD3D11*>(CInstance::Get().GetCurrDevice());
    CommandList = std::make_unique<CCommandListD3D11>(DeviceImpl);
}

void CDrawPass::Impl::BeginRecording()
{
    CommandList->BeginRecording();

    //Bind render targets
    int vpWidth = 0, vpHeight = 0;

    std::vector<CImageView*> colors;
    CImageView* depthStencil = nullptr;

    CNodeId depthId = Owner.GetDepthStencilAttachment();
    if (depthId != kInvalidNodeId)
    {
        auto& attachment = Owner.GetRenderGraph().GetRenderTarget(depthId);
        auto imageView = attachment.GetImageView();
        depthStencil = imageView;
    }

    Owner.ForEachColorAttachment([&](CNodeId id)
    {
        if (id == kInvalidNodeId)
            return;
        auto& attachment = Owner.GetRenderGraph().GetRenderTarget(id);
        auto imageView = attachment.GetImageView();
        colors.push_back(imageView);

        if (attachment.IsSwapChain())
            attachment.GetDimensions(vpWidth, vpHeight);
    });

    //Try with default viewport
    D3D11_VIEWPORT vp;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = (FLOAT)vpWidth;
    vp.Height = (FLOAT)vpHeight;
    vp.MinDepth = 0;
    vp.MaxDepth = 1;
    CommandList->SetDefaultViewport(vp);

    CommandList->SetRenderTargets(colors, depthStencil);
}

void CDrawPass::Impl::Record(CPipelineStates states, const CDrawTemplate& drawTemplate)
{
    CommandList->Draw(states, drawTemplate);
}

void CDrawPass::Impl::FinishRecording()
{
    CommandList->FinishRecording();
}

void CDrawPass::Impl::Submit()
{
    //Submit
    DeviceImpl->ImmediateContext->ExecuteCommandList(CommandList->GetD3DCommandList(), false);
    CommandList->ClearD3DCommandList();
}

///////////////////////////////////////////////////////////////////////////////
// CClearPass
///////////////////////////////////////////////////////////////////////////////

CClearPass::Impl::Impl(CClearPass& owner) : Owner(owner)
{
    DeviceImpl = static_cast<CDeviceD3D11*>(CInstance::Get().GetCurrDevice());
}

void CClearPass::Impl::Submit()
{
    CNodeId depthId = Owner.GetDepthStencilAttachment();
    if (depthId != kInvalidNodeId)
    {
        auto& attachment = Owner.GetRenderGraph().GetRenderTarget(depthId);
        auto imageView = attachment.GetImageView();
        auto* imageViewImpl = static_cast<CImageViewD3D11*>(imageView.Get());
        ComPtr<ID3D11DepthStencilView> dsv = imageViewImpl->GetDepthStencilView();
        DeviceImpl->ImmediateContext->ClearDepthStencilView(dsv.Get(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, Owner.ClearDepth, Owner.ClearStencil);
    }

    int i = 0;
    Owner.ForEachColorAttachment([&](CNodeId id)
    {
        if (id == kInvalidNodeId)
            return;
        auto& attachment = Owner.GetRenderGraph().GetRenderTarget(id);
        auto imageView = attachment.GetImageView();
        auto* imageViewImpl = static_cast<CImageViewD3D11*>(imageView.Get());
        auto rtv = imageViewImpl->GetRenderTargetView();
        DeviceImpl->ImmediateContext->ClearRenderTargetView(rtv.Get(), Owner.ClearColors[i].data());
        i++;
    });
}

} /* namespace RHI */
