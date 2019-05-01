#include "SubmissionTracker.h"
#include "CommandContextVk.h"
#include "DeviceVk.h"

namespace RHI
{

CSubmissionTracker::CSubmissionTracker(CDeviceVk& p)
    : Parent(p)
{
    // Don't do anything here, since this is called before the device is initialized
}

void CSubmissionTracker::Init()
{
    for (uint32_t i = 0; i < MaxFramesInFlight; i++)
    {
        CFrameResources& r = FrameResources[i];
        r.FrameCmdPool = std::make_shared<CCommandPoolVk>(Parent, QT_GRAPHICS, false);
    }
    CurrentFrameResourcesIndex = 0;
}

void CSubmissionTracker::Shutdown()
{
    VK(vkDeviceWaitIdle(Parent.GetVkDevice()));

    while (!JobQueue.empty())
        PopFrontJob(true);

    for (auto& pair : TransientPools)
        for (auto& pair2 : pair.second)
        {
            pair2.second.reset();
        }
    TransientPools.clear();

    for (uint32_t i = 0; i < MaxFramesInFlight; i++)
    {
        CFrameResources& r = FrameResources[i];
        r.FrameCmdPool.reset();
    }
}

void CSubmissionTracker::SubmitJob(CGPUJobInfo jobInfo, bool wait)
{
    std::unique_lock<std::mutex> lk(JobSubmitMutex);

    // Wait if the queue is too full
    while (JobQueue.size() >= MaxJobsInFlight
           || (jobInfo.bIsFrameJob && FrameJobCount >= MaxFramesInFlight))
        PopFrontJob(true);

    // Clear those finished jobs
    while (!JobQueue.empty()
           && vkGetFenceStatus(Parent.GetVkDevice(), JobQueue.front().Fence) == VK_SUCCESS)
        PopFrontJob();

    if (jobInfo.bIsFrameJob)
    {
        Parent.GetHugeConstantBuffer()->MarkBlockEnd();
        jobInfo.FrameResourceIndex = CurrentFrameResourcesIndex;
        GetCurrentFrameResources().InFlightLock.Lock();
        FrameJobCount++;
        CurrentFrameResourcesIndex++;
        CurrentFrameResourcesIndex %= MaxFramesInFlight;
    }

    std::vector<VkCommandBuffer> cmdBufferHandles;
    for (const auto& ptr : jobInfo.CmdBuffers)
        cmdBufferHandles.push_back(ptr->GetHandle());

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = static_cast<uint32_t>(cmdBufferHandles.size());
    submitInfo.pCommandBuffers = cmdBufferHandles.data();
    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(jobInfo.WaitSemaphores.size());
    submitInfo.pWaitSemaphores = jobInfo.WaitSemaphores.data();
    submitInfo.pWaitDstStageMask = jobInfo.WaitStages.data();
    if (!jobInfo.SignalSemaphores.empty())
    {
        submitInfo.signalSemaphoreCount = static_cast<uint32_t>(jobInfo.SignalSemaphores.size());
        submitInfo.pSignalSemaphores = jobInfo.SignalSemaphores.data();
    }

    VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    VK(vkCreateFence(Parent.GetVkDevice(), &fenceInfo, nullptr, &jobInfo.Fence));

    VkQueue q = Parent.GetVkQueue(jobInfo.QueueType);
    VK(vkQueueSubmit(q, 1, &submitInfo, jobInfo.Fence));

    // Queue up
    std::swap(jobInfo.PendingDeletionBuffers, Parent.PendingDeletionBuffers);
    std::swap(jobInfo.PendingDeletionImages, Parent.PendingDeletionImages);
    if (wait)
    {
        JobQueue.push_front(std::move(jobInfo));
        PopFrontJob(true);
    }
    else
        JobQueue.push_back(std::move(jobInfo));
}

void CSubmissionTracker::PopFrontJob(bool wait)
{
    auto& waitJob = JobQueue.front();

    VkResult result;
    if (wait)
    {
        result = vkWaitForFences(Parent.GetVkDevice(), 1, &waitJob.Fence, VK_TRUE, 1000000000);
        if (result != VK_SUCCESS)
            throw CRHIRuntimeError("vkWaitForFences timedout or errored: "
                                   + std::to_string(result));
    }

    if (waitJob.bIsFrameJob)
    {
        Parent.GetHugeConstantBuffer()->FreeBlock();
        FrameResources[waitJob.FrameResourceIndex].InFlightLock.Unlock();
        waitJob.CmdBuffers.clear();
        FrameResources[waitJob.FrameResourceIndex].FrameCmdPool->ResetPool();
        FrameJobCount--;
    }

    vkDestroyFence(Parent.GetVkDevice(), waitJob.Fence, nullptr);
    for (VkSemaphore semaphore : waitJob.SignalSemaphores)
        vkDestroySemaphore(Parent.GetVkDevice(), semaphore, nullptr); // SAFE?

    for (auto& fn : waitJob.DeferredDeleters)
        fn();
    for (auto& pair : waitJob.PendingDeletionBuffers)
        vmaDestroyBuffer(Parent.GetAllocator(), pair.first, pair.second);
    for (auto& pair : waitJob.PendingDeletionImages)
        vmaDestroyImage(Parent.GetAllocator(), pair.first, pair.second);

    JobQueue.pop_front();
}

CCommandPoolVk::Ref CSubmissionTracker::GetTransientPool(EQueueType queueType)
{
    auto id = std::this_thread::get_id();
    auto iter = TransientPools.find(id);
    if (iter == TransientPools.end())
    {
        // Create a new pool
        TransientPools[id][queueType] = std::make_shared<CCommandPoolVk>(Parent, queueType);
        return TransientPools[id][queueType];
    }

    auto iter2 = iter->second.find(queueType);
    if (iter2 == iter->second.end())
    {
        TransientPools[id][queueType] = std::make_shared<CCommandPoolVk>(Parent, queueType);
        return TransientPools[id][queueType];
    }

    return iter2->second;
}

CFrameResources& CSubmissionTracker::GetCurrentFrameResources()
{
    while (FrameJobCount >= MaxFramesInFlight)
        PopFrontJob(true);

    auto& r = FrameResources[CurrentFrameResourcesIndex];
    // Make sure this resource struct isn't in use by the GPU
    r.InFlightLock.Lock();
    r.InFlightLock.Unlock();
    return r;
}

}
