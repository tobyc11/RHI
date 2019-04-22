#pragma once
#include "VkCommon.h"
#include <SpinLock.h>
#include <array>
#include <deque>
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <thread>

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

enum EQueueType
{
    QT_TRANSFER,
    QT_COMPUTE,
    QT_GRAPHICS,
    NUM_QUEUE_TYPES
};

// Global resources swapped every frame
struct CFrameResources
{
    // This struct is locked when submitted
    tc::FSpinLock InFlightLock;
    VkCommandPool CommandPool;
    std::vector<VkCommandBuffer> CommandBuffers;
    uint32_t NextFreeCommandBuffer;

    VkCommandBuffer AllocateCmdBuffer() { return CommandBuffers[NextFreeCommandBuffer++]; }
};

struct CGPUJobInfo
{
    CGPUJobInfo(const std::vector<VkCommandBuffer>& cmdBuffers,
                std::vector<VkSemaphore> waitSemaphores,
                std::vector<VkPipelineStageFlags> waitStages,
                std::vector<VkSemaphore> signalSemaphores, EQueueType queueType,
                std::vector<std::function<void()>> deferredDeleters)
        : CmdBuffers(cmdBuffers)
        , WaitSemaphores(std::move(waitSemaphores))
        , WaitStages(std::move(waitStages))
        , SignalSemaphores(std::move(signalSemaphores))
        , QueueType(queueType)
        , DeferredDeleters(std::move(deferredDeleters))
    {
    }

    void SetImmediateJob(bool frameEnd = false)
    {
        Kind = ECommandContextKind::Immediate;
        bIsFrameJob = frameEnd;
    }

    void SetTransientJob() { Kind = ECommandContextKind::Transient; }

    void SetDeferredJob(std::shared_ptr<CCommandContextVk> ctx)
    {
        Kind = ECommandContextKind::Deferred;
        DeferredContext = ctx;
    }

    ECommandContextKind Kind = ECommandContextKind::Invalid;
    std::vector<VkCommandBuffer> CmdBuffers;
    bool bIsFrameJob = false;
    std::shared_ptr<CCommandContextVk> DeferredContext;
    std::vector<VkSemaphore> WaitSemaphores;
    std::vector<VkPipelineStageFlags> WaitStages;
    std::vector<VkSemaphore> SignalSemaphores;
    EQueueType QueueType = QT_GRAPHICS;
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

    VkCommandPool GetTransientPool(EQueueType queueType);
    CFrameResources& GetCurrentFrameResources();

private:
    CDeviceVk& Parent;

    // A ring buffer contains the jobs currently in flight
    std::mutex JobSubmitMutex;
    std::deque<CGPUJobInfo> JobQueue;
    uint32_t FrameJobCount = 0;
    static const uint32_t MaxJobsInFlight = 8;
    static const uint32_t MaxFramesInFlight = 2;

    // I envisage three kinds of pools: frame pool, transient pool, and deferred pool
    //   We have one transient pool per thread
    //   Frame pool goes with the immediate context, but is returned here every frame
    //   Deferred pool is managed by the context, and reference counted
    std::unordered_map<std::thread::id, std::map<EQueueType, VkCommandPool>> TransientPools;

    std::array<CFrameResources, MaxFramesInFlight> FrameResources;
    uint32_t CurrentFrameResourcesIndex;
};

}
