#pragma once
#include "CommandQueue.h"
#include "MtlCommon.h"

#ifdef __OBJC__
#include <dispatch/dispatch.h>
#endif

namespace RHI
{

class CCommandQueueMetal : public CCommandQueue
{
public:
    typedef std::shared_ptr<CCommandQueueMetal> Ref;

    CCommandQueueMetal(CDeviceMetal& parent);
    ~CCommandQueueMetal() override;

    CCommandList::Ref CreateCommandList() override;
    void Flush() override;
    void Finish() override;

    CDeviceMetal& GetDevice() const { return Parent; }
    id GetMTLQueue() const { return Queue; }

    void DrainCleanupCallbacks();
    void EnqueuePendingBuffer(id cmdBuf);

private:
    static const int FrameCount = 3;

    CDeviceMetal& Parent;
    id Queue;
#ifdef __OBJC__
    dispatch_semaphore_t FrameSemaphore;
#else
    void* FrameSemaphore;
#endif

    std::vector<id> PendingCommandBuffers;
};

} /* namespace RHI */
