#pragma once
#include "Image.h"
#include "RenderPass.h"

namespace Nome::RHI
{

//I am the frame.
class CRenderGraph
{
public:
    CNodeId ImportRenderTarget(CImageView* view);
    CNodeId ImportRenderTarget(CSwapChain* swapChain);
    CNodeId DeclareRenderTarget();

    CRenderPass& AddRenderPass(CNodeId id, const std::vector<CNodeId>& outputTargets, CNodeId depthStencilTarget);
    CRenderPass& GetRenderPass(CNodeId id);

    void BeginFrame();
    void SubmitFrame();

private:
    //Sole ownership
    std::map<CNodeId, CRenderPass*> RenderPasses;

    std::map<CNodeId, CRenderTargetRef> RenderTargets;
};

} /* namespace Nome::RHI */
