#include "RenderGraph.h"
#include "RHIException.h"

namespace RHI
{

void CRenderGraph::ImportRenderTarget(CNodeId id, CImageView* view)
{
    throw std::runtime_error("unimplemented");
}

void CRenderGraph::ImportRenderTarget(CNodeId id, CSwapChain* swapChain)
{
    RenderTargets.insert_or_assign(id, CRenderTargetRef(swapChain));
}

CNodeId CRenderGraph::DeclareRenderTarget()
{
    //Try to generate an unused id
    CNodeId id;
    do
    {
        //https://burtleburtle.net/bob/hash/integer.html
        id = (id + 0x7ed55d16) + (id << 12);
        id = (id ^ 0xc761c23c) ^ (id >> 19);
        id = (id + 0x165667b1) + (id << 5);
        id = (id + 0xd3a2646c) ^ (id << 9);
        id = (id + 0xfd7046c5) + (id << 3);
        id = (id ^ 0xb55a4f09) ^ (id >> 16);
        if (id == 0)
            id++;
    } while (RenderTargets.find(id) != RenderTargets.end());
    //Register a new and empty render target
    RenderTargets.emplace(id, CRenderTargetRef());
    return id;
}

CRenderTargetRef& CRenderGraph::GetRenderTarget(CNodeId id)
{
    auto iter = RenderTargets.find(id);
    if (iter != RenderTargets.end())
        return iter->second;
    throw CRHIRuntimeError("No render target with given id found");
}

void CRenderGraph::AddRenderPass(CNodeId id, CRenderPass& pass)
{
    sp<CRenderPass> ptr = &pass;
    RenderPasses.emplace(id, ptr);
}

bool CRenderGraph::HasRenderPass(CNodeId id) const
{
    if (RenderPasses.find(id) == RenderPasses.end())
        return false;
    return true;
}

CRenderPass& CRenderGraph::GetRenderPass(CNodeId id)
{
    auto iter = RenderPasses.find(id);
    if (iter == RenderPasses.end())
        throw CRHIRuntimeError("Could not find specified render target");
    return *iter->second;
}

void CRenderGraph::RemoveRenderPass(CNodeId id)
{
    auto iter = RenderPasses.find(id);
    RenderPasses.erase(iter);
}

void CRenderGraph::BeginFrame()
{
    //Go through all the render targets and make sure the swapchains are properly setup
    for (auto& pair : RenderTargets)
    {
        if (pair.second.IsSwapChain())
            pair.second.SetImageViewFromSwapChain();
    }
}

void CRenderGraph::SubmitFrame()
{
    //TODO: this needs to be multithreaded
    DFSVisited.clear();
    TopoSortedPasses.clear();
    TopoSortedPasses.reserve(RenderPasses.size());

    for (const auto& pair : RenderPasses)
    {
        DFSRenderPassTopologicalSort(pair.second.Get());
    }

    //TODO: this probably only works with legacy rendering APIs as well
    for (auto iter = TopoSortedPasses.rbegin(); iter != TopoSortedPasses.rend(); ++iter)
    {
        (*iter)->Submit();
    }
}

void CRenderGraph::PrepareToResize()
{
    for (auto& pair : RenderTargets)
    {
        if (pair.second.IsSwapChain())
            pair.second.ClearImageAndView();
    }
}

void CRenderGraph::DFSRenderPassTopologicalSort(CRenderPass* pass)
{
    if (DFSVisited.find(pass) != DFSVisited.end())
    {
        //Already visited
        return;
    }

    DFSVisited.insert(pass);
    for (CRenderPass* successor : pass->Succ)
    {
        DFSRenderPassTopologicalSort(successor);
    }

    TopoSortedPasses.push_back(pass);
}

} /* namespace RHI */
