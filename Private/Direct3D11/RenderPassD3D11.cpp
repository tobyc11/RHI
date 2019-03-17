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
}

void CDrawPass::Impl::Record(const CDrawTemplate& drawTemplate)
{
    auto entry = CommandList->CacheDrawCall(drawTemplate);
    CommandList->Draw(entry);
    OneShotEntries.insert(entry);
}

void CDrawPass::Impl::Record(CNodeId id, const CDrawTemplate& drawTemplate)
{
    //Return if the id is already in the cache, if changed, pls delete and re-record
    if (IdToCacheEntry.find(id) != IdToCacheEntry.end())
        return;

    auto cacheEntry = CommandList->CacheDrawCall(drawTemplate);
    IdToCacheEntry[id] = cacheEntry;
    CommandList->Draw(cacheEntry);
}

void CDrawPass::Impl::DeleteRecord(CNodeId id)
{
    CommandList->RemoveCachedDrawCall(IdToCacheEntry[id]);
    IdToCacheEntry.erase(id);
}

void CDrawPass::Impl::FinishRecording()
{
    //Draw all the cached records
    for (auto& pair : IdToCacheEntry)
        CommandList->Draw(pair.second);

    CommandList->FinishRecording();
}

void CDrawPass::Impl::Submit()
{
    //Bind render targets
    int vpWidth = 0, vpHeight = 0;

    ComPtr<ID3D11DepthStencilView> DSV;
    CNodeId depthId = Owner.GetDepthStencilAttachment();
    if (depthId != kInvalidNodeId)
    {
        auto& attachment = Owner.GetRenderGraph().GetRenderTarget(depthId);
        auto imageView = attachment.GetImageView();
        auto* imageViewImpl = static_cast<CImageViewD3D11*>(imageView.Get());
        DSV = imageViewImpl->GetDepthStencilView();
    }

    std::vector<ID3D11RenderTargetView*> RTVs;
    Owner.ForEachColorAttachment([&](CNodeId id)
    {
        if (id == kInvalidNodeId)
            return;
        auto& attachment = Owner.GetRenderGraph().GetRenderTarget(id);
        auto imageView = attachment.GetImageView();
        auto* imageViewImpl = static_cast<CImageViewD3D11*>(imageView.Get());
        auto rtv = imageViewImpl->GetRenderTargetView();
        RTVs.push_back(rtv.Get());

        if (attachment.IsSwapChain())
            attachment.GetDimensions(vpWidth, vpHeight);
    });

    DeviceImpl->ImmediateContext->OMSetRenderTargets((UINT)RTVs.size(), RTVs.data(), DSV.Get());

    //Try with default viewport
    //D3D11_VIEWPORT vp;
    //vp.TopLeftX = 0;
    //vp.TopLeftY = 0;
    //vp.Width = vpWidth;
    //vp.Height = vpHeight;
    //vp.MinDepth = 0;
    //vp.MaxDepth = 1;
    //DeviceImpl->ImmediateContext->RSSetViewports(1, &vp);
    //DeviceImpl->ImmediateContext->RSSetScissorRects(0, nullptr);

    //Submit
    DeviceImpl->ImmediateContext->ExecuteCommandList(CommandList->GetD3DCommandList(), false);

    //Cleanup
    for (const auto& entry : OneShotEntries)
        CommandList->RemoveCachedDrawCall(entry);
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
