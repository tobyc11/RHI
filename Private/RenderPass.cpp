#include "RenderPass.h"
#include "RenderGraph.h"
#include "RHIInstance.h"
#include "RHIException.h"
#ifdef RHI_IMPL_DIRECT3D11
#include "Direct3D11/CommandListD3D11.h"
#endif
#include <unordered_map>

namespace RHI
{

///////////////////////////////////////////////////////////////////////////////
// CRenderTargetRef
///////////////////////////////////////////////////////////////////////////////

CRenderTargetRef::CRenderTargetRef() : Width(0), Height(0)
{
}

CRenderTargetRef::CRenderTargetRef(CSwapChain* swapChain)
    : SwapChain(swapChain)
{
    SwapChain->GetSize(Width, Height);
}

void CRenderTargetRef::GetDimensions(int& outWidth, int& outHeight)
{
    if (IsSwapChain())
    {
        SwapChain->GetSize(outWidth, outHeight);
        return;
    }
    outWidth = Width;
    outHeight = Height;
}

void CRenderTargetRef::SetDimensions(int width, int height)
{
    Width = width;
    Height = height;
}

void CRenderTargetRef::SetImageViewFromSwapChain()
{
    bool doit = false;
    int currWidth, currHeight;
    SwapChain->GetSize(currWidth, currHeight);
    if (Width != currWidth || Height != currHeight)
        doit = true;
    Width = currWidth;
    Height = currHeight;
    if (!ImageView || !Image)
        doit = true;

    if (!Image)
        Image = SwapChain->GetImage(0);

    CImageViewDesc desc;
    desc.Format = EFormat::R8G8B8A8_UNORM; //TODO: change this
    desc.Type = EImageViewType::View2D;
    desc.Range.BaseMipLevel = 0;
    desc.Range.LevelCount = 1;
    desc.Range.BaseArrayLayer = 0;
    desc.Range.LayerCount = 1;

    ImageView = CInstance::Get().GetCurrDevice()->CreateImageView(desc, Image);
}

void CRenderTargetRef::InitDepthStencilImage()
{
    auto dev = CInstance::Get().GetCurrDevice();
    Image = dev->CreateImage2D(EFormat::D24_UNORM_S8_UINT, EImageUsageFlags::DepthStencil, Width, Height);
}

void CRenderTargetRef::ClearImageAndView()
{
    ImageView.Clear();
    Image.Clear();
}

///////////////////////////////////////////////////////////////////////////////
// CRenderPass
///////////////////////////////////////////////////////////////////////////////

void CRenderPass::SetColorAttachment(uint32_t index, CNodeId id)
{
    ColorAttachments[index] = id;
}

void CRenderPass::SetDepthStencilAttachment(CNodeId id)
{
    DepthStencilAttachment = id;
}

void CRenderPass::AddDependency(CRenderPass& predecessor)
{
    auto iter = Pred.find(&predecessor);
    if (iter != Pred.end())
        return;
    Pred.insert(&predecessor);
    predecessor.Succ.insert(this);
}

void CRenderPass::RemoveDependency(CRenderPass& predecessor)
{
    auto iter = Pred.find(&predecessor);
    if (iter == Pred.end())
        return;
    Pred.erase(iter);
    predecessor.Succ.erase(this);
}

void CRenderPass::Submit() const
{
}

///////////////////////////////////////////////////////////////////////////////
// Draw pass
///////////////////////////////////////////////////////////////////////////////

class CDrawPassPriv
{
public:
    //TODO: this is currently d3d11 dependent, make this more general
    sp<CDevice> Device;
    std::unique_ptr<CCommandListD3D11> CommandList;
    std::unordered_map<CNodeId, CCommandListD3D11::CDrawCallCacheEntryRef> CachedDrawCalls;
};

CDrawPass::CDrawPass(CRenderGraph& renderGraph) : CRenderPass(renderGraph)
{
    Priv = new CDrawPassPriv();
    Priv->Device = CInstance::Get().GetCurrDevice();
    auto* deviceImpl = static_cast<CDeviceD3D11*>(Priv->Device.Get());
    Priv->CommandList = std::make_unique<CCommandListD3D11>(deviceImpl);
}

CDrawPass::~CDrawPass()
{
    delete Priv;
}

void CDrawPass::DrawOnce(const CDrawTemplate& drawInfo)
{
    throw std::runtime_error("unimplemented");
}

void CDrawPass::AddOrUpdateDraw(CNodeId id, const CDrawTemplate& drawInfo)
{
    auto iter = Priv->CachedDrawCalls.find(id);
    if (iter != Priv->CachedDrawCalls.end())
    {
        Priv->CommandList->RemoveCachedDrawCall(iter->second);
    }
    auto cacheRef = Priv->CommandList->CacheDrawCall(drawInfo);
    Priv->CachedDrawCalls.emplace(id, cacheRef);

    Priv->CommandList->Draw(cacheRef);
}

void CDrawPass::TouchDraw(CNodeId id)
{
    auto iter = Priv->CachedDrawCalls.find(id);
    if (iter != Priv->CachedDrawCalls.end())
    {
        Priv->CommandList->Draw(iter->second);
        return;
    }
    throw CRHIRuntimeError("id does not exist in cached draw calls");
}

void CDrawPass::GarbageCollect()
{
}

///////////////////////////////////////////////////////////////////////////////
// CPresentPass
///////////////////////////////////////////////////////////////////////////////

CPresentPass::CPresentPass(CRenderGraph& renderGraph) : CRenderPass(renderGraph)
{
}

void CPresentPass::Submit() const
{
    ForEachColorAttachment([this](CNodeId attachment)
    {
        if (attachment == kInvalidNodeId)
            return;

        auto& target = GetRenderGraph().GetRenderTarget(attachment);
        if (target.IsSwapChain())
        {
            target.GetSwapChain()->Present();
        }
    });
}

} /* namespace RHI */
