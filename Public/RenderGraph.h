#pragma once
#include "Image.h"
#include "RenderPass.h"

namespace RHI
{

//I am the frame.
class CRenderGraph
{
public:
    void ImportRenderTarget(CNodeId id, CImageView* view);
    void ImportRenderTarget(CNodeId id, CSwapChain* swapChain);
    CNodeId DeclareRenderTarget();
    CRenderTargetRef& GetRenderTarget(CNodeId id);

    void AddRenderPass(CNodeId id, CRenderPass& pass);
    bool HasRenderPass(CNodeId id) const;
    CRenderPass& GetRenderPass(CNodeId id);
    void RemoveRenderPass(CNodeId id);

    void BeginFrame();
    void SubmitFrame();
    void PrepareToResize();

    bool IsDuringFrame() const { return bIsDuringFrame; }

private:
    void DFSRenderPassTopologicalSort(CRenderPass* pass);

    //Sole ownership
    std::map<CNodeId, sp<CRenderPass>> RenderPasses;

    std::map<CNodeId, CRenderTargetRef> RenderTargets;

    std::set<CRenderPass*> DFSVisited;
    std::vector<CRenderPass*> TopoSortedPasses;

    bool bIsDuringFrame = false;
};

} /* namespace RHI */
