#include "CommandQueueMetal.h"
#include "CommandListMetal.h"
#include "DeviceMetal.h"
#include "RHIException.h"

namespace RHI
{

CCommandQueueMetal::CCommandQueueMetal(CDeviceMetal& parent)
    : Parent(parent)
{
    Queue = [(id<MTLDevice>)Parent.GetMTLDevice() newCommandQueue];
    if (!Queue)
        throw CRHIRuntimeError("Failed to create Metal command queue");

    FrameSemaphore = dispatch_semaphore_create(FrameCount);
}

CCommandQueueMetal::~CCommandQueueMetal() { }

CCommandList::Ref CCommandQueueMetal::CreateCommandList()
{
    return std::make_shared<CCommandListMetal>(*this);
}

void CCommandQueueMetal::EnqueuePendingBuffer(id cmdBuf)
{
    PendingCommandBuffers.push_back(cmdBuf);
}

void CCommandQueueMetal::Flush()
{
    for (auto& cmdBuf : PendingCommandBuffers)
    {
        [(id<MTLCommandBuffer>)cmdBuf commit];
    }
    PendingCommandBuffers.clear();
}

void CCommandQueueMetal::Finish()
{
    for (auto& cmdBuf : PendingCommandBuffers)
    {
        [(id<MTLCommandBuffer>)cmdBuf commit];
        [(id<MTLCommandBuffer>)cmdBuf waitUntilCompleted];
    }
    PendingCommandBuffers.clear();

    DrainCleanupCallbacks();
}

void CCommandQueueMetal::DrainCleanupCallbacks()
{
    std::vector<std::function<void(CDeviceMetal&)>> callbacks;
    {
        std::lock_guard<std::mutex> lock(Parent.DeviceMutex);
        callbacks.swap(Parent.PostFrameCleanup);
    }
    for (auto& cb : callbacks)
        cb(Parent);
}

} /* namespace RHI */
