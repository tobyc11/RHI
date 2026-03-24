#include "CommandListMetal.h"
#include "CommandContextMetal.h"
#include "CommandQueueMetal.h"
#include "RHIException.h"
#include "RenderPassMetal.h"

namespace RHI
{

CCommandListMetal::CCommandListMetal(CCommandQueueMetal& queue)
    : Queue(queue)
{
    CommandBuffer = [(id<MTLCommandQueue>)queue.GetMTLQueue() commandBuffer];
    if (!CommandBuffer)
        throw CRHIRuntimeError("Failed to create Metal command buffer");
}

CCommandListMetal::~CCommandListMetal() { }

void CCommandListMetal::Enqueue()
{
    [(id<MTLCommandBuffer>)CommandBuffer enqueue];
}

void CCommandListMetal::Commit()
{
    Queue.EnqueuePendingBuffer(CommandBuffer);
}

ICopyContext::Ref CCommandListMetal::CreateCopyContext()
{
    return CCommandContextMetal::CreateCopyContext(*this);
}

IComputeContext::Ref CCommandListMetal::CreateComputeContext()
{
    return CCommandContextMetal::CreateComputeContext(*this);
}

IParallelRenderContext::Ref CCommandListMetal::CreateParallelRenderContext(
    CRenderPass::Ref renderPass, const std::vector<CClearValue>& clearValues)
{
    return std::make_shared<CRenderPassContextMetal>(*this, renderPass, clearValues);
}

} /* namespace RHI */
