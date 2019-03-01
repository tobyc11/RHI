#pragma once
#include "DrawTemplate.h"
#include "SwapChain.h"

namespace Nome::RHI
{

using CNodeId = uint32_t;

class CRenderGraph;

class CRenderTargetRef
{
public:

private:
    bool bIsSwapChain;
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
