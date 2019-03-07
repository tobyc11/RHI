#include "RenderGraph.h"

namespace Nome::RHI
{

CNodeId CRenderGraph::ImportRenderTarget(CImageView* view)
{
    throw std::runtime_error("unimplemented");
}

CNodeId CRenderGraph::ImportRenderTarget(CSwapChain* swapChain)
{
    RenderTargets.emplace(RenderTargetCount, swapChain);
    return RenderTargetCount++;
}

CNodeId CRenderGraph::DeclareRenderTarget()
{
    throw std::runtime_error("unimplemented");
}

CRenderPass& CRenderGraph::AddRenderPass(CNodeId id, const std::vector<CNodeId>& outputTargets, CNodeId depthStencilTarget)
{
    // TODO: insert return statement here
    throw std::runtime_error("unimplemented");
}

CRenderPass& CRenderGraph::GetRenderPass(CNodeId id)
{
    // TODO: insert return statement here
    throw std::runtime_error("unimplemented");
}

void CRenderGraph::BeginFrame()
{
}

void CRenderGraph::SubmitFrame()
{
}

} /* namespace Nome::RHI */
