#pragma once
#include "DrawTemplate.h"
#include "SwapChain.h"

namespace Nome::RHI
{

//We use an integer type to uniquely identify graph nodes and other sorts objects
//  __hash should be used to convert a string literal into an integer id.
using CNodeId = uint32_t;

class CRenderGraph;

class CRenderTargetRef
{
public:
    CRenderTargetRef(CSwapChain* swapChain);

private:
    bool bIsSwapChain = false;
    sp<CSwapChain> SwapChain;
};

class CRenderPass
{
    friend class CRenderGraph;

public:
    CRenderPass(CRenderGraph& renderGraph);

    void DrawOnce(const CDrawTemplate& drawInfo);
    void AddDraw(CNodeId id, const CDrawTemplate& drawInfo);
    void UpdateDraw(CNodeId id, const CDrawTemplate& drawInfo);

private:
    CRenderGraph& RenderGraph;
};

} /* namespace Nome::RHI */
