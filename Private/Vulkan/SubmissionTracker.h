#pragma once
#include "CommandBufferVk.h"
#include "VkCommon.h"
#include <SpinLock.h>
#include <array>
#include <deque>
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>

namespace RHI
{

// Each kind is associated with a different command pool management policy
enum class ECommandContextKind : uint32_t
{
    Invalid,
    Immediate,
    Transient,
    Deferred
};

// Global resources swapped every frame
struct CFrameResources
{
    // This struct is locked when submitted
    tc::FSpinLock InFlightLock;
    CCommandPoolVk::Ref FrameCmdPool;
};

struct CGPUJobInfo
{
    CGPUJobInfo(std::vector<std::unique_ptr<CCommandBufferVk>> cmdBuffers, bool frameJob,
                std::vector<VkSemaphore> waitSemaphores,
                std::vector<VkPipelineStageFlags> waitStages,
                std::vector<VkSemaphore> signalSemaphores, EQueueType queueType,
                std::vector<std::function<void()>> deferredDeleters)
        : CmdBuffers(std::move(cmdBuffers))
        , bIsFrameJob(frameJob)
        , WaitSemaphores(std::move(waitSemaphores))
        , WaitStages(std::move(waitStages))
        , SignalSemaphores(std::move(signalSemaphores))
        , QueueType(queueType)
        , DeferredDeleters(std::move(deferredDeleters))
    {
    }

    std::vector<std::unique_ptr<CCommandBufferVk>> CmdBuffers;
    bool bIsFrameJob = false;
    std::vector<VkSemaphore> WaitSemaphores;
    std::vector<VkPipelineStageFlags> WaitStages;
    std::vector<VkSemaphore> SignalSemaphores;
    EQueueType QueueType;
    // Deleters that delete the transient resources only used by this command buffer
    std::vector<std::function<void()>> DeferredDeleters;

private:
    friend class CSubmissionTracker;

    VkFence Fence;
    std::vector<std::pair<VkBuffer, VmaAllocation>> PendingDeletionBuffers;
    std::vector<std::pair<VkImage, VmaAllocation>> PendingDeletionImages;
    uint32_t FrameResourceIndex;
};

class CSubmissionTracker
{
public:
    CSubmissionTracker(CDeviceVk& p);

    void Init();
    void Shutdown();

    void SubmitJob(CGPUJobInfo jobInfo, bool wait = false);
    void PopFrontJob(bool wait = false);

    CCommandPoolVk::Ref GetTransientPool(EQueueType queueType);
    CFrameResources& GetCurrentFrameResources();

private:
    CDeviceVk& Parent;

    // A ring buffer contains the jobs currently in flight
    std::mutex JobSubmitMutex;
    std::deque<CGPUJobInfo> JobQueue;
    uint32_t FrameJobCount = 0;
    static const uint32_t MaxJobsInFlight = 8;
    static const uint32_t MaxFramesInFlight = 3;

    // I envisage three kinds of pools: frame pool, transient pool, and deferred pool
    //   We have one transient pool per thread
    //   Frame pool goes with the immediate context, but is returned here every frame
    //   Deferred pool is managed by the context, and reference counted
    std::unordered_map<std::thread::id, std::map<EQueueType, CCommandPoolVk::Ref>> TransientPools;

    std::array<CFrameResources, MaxFramesInFlight> FrameResources;
    uint32_t CurrentFrameResourcesIndex;
};

}
