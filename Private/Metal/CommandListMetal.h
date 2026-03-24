#pragma once
#include "CommandQueue.h"
#include "MtlCommon.h"

namespace RHI
{

class CCommandQueueMetal;

class CCommandListMetal : public CCommandList
{
public:
    typedef std::shared_ptr<CCommandListMetal> Ref;

    CCommandListMetal(CCommandQueueMetal& queue);
    ~CCommandListMetal() override;

    void Enqueue() override;
    void Commit() override;

    ICopyContext::Ref CreateCopyContext() override;
    IComputeContext::Ref CreateComputeContext() override;
    IParallelRenderContext::Ref CreateParallelRenderContext(
        CRenderPass::Ref renderPass, const std::vector<CClearValue>& clearValues) override;

    id GetMTLCommandBuffer() const { return CommandBuffer; }
    CCommandQueueMetal& GetQueue() const { return Queue; }

private:
    CCommandQueueMetal& Queue;
    id CommandBuffer;
};

} /* namespace RHI */
