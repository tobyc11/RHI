#pragma once
#include "RHICommon.h"
#include "RHIChooseImpl.h"
#include "CopyContext.h"
#include "ComputeContext.h"
#include "RenderContext.h"
#include <memory>

namespace RHI
{

enum class EQueueType : uint32_t
{
    Copy,
    Compute,
    Render,
    Count
};

class CCommandList : public std::enable_shared_from_this<CCommandList>
{
public:
    typedef std::shared_ptr<CCommandList> Ref;

    virtual ~CCommandList() = default;

    virtual void Enqueue() = 0;
    virtual void Commit() = 0;

    virtual ICopyContext::Ref CreateCopyContext() = 0;
    virtual IComputeContext::Ref CreateComputeContext() = 0;
    virtual IParallelRenderContext::Ref CreateParallelRenderContext(CRenderPass::Ref renderPass,
                                                                    const std::vector<CClearValue>& clearValues) = 0;
};

class CCommandQueue : public std::enable_shared_from_this<CCommandQueue>
{
public:
    typedef std::shared_ptr<CCommandQueue> Ref;

    virtual ~CCommandQueue() = default;

    virtual CCommandList::Ref CreateCommandList() = 0;

    virtual void Flush() = 0;
    virtual void Finish() = 0;
};

} /* namespace RHI */
