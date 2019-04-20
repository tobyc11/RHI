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
    VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    poolInfo.flags = 0; // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT?
    poolInfo.queueFamilyIndex = Parent.GetQueueFamily(QT_GRAPHICS);

    for (uint32_t i = 0; i < MaxFramesInFlight; i++)
    {
        CFrameResources& r = FrameResources[i];
        vkCreateCommandPool(Parent.GetVkDevice(), &poolInfo, nullptr, &r.CommandPool);

        VkCommandBufferAllocateInfo allocInfo;
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.commandPool = r.CommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        r.CommandBuffers.resize(4);
        allocInfo.commandBufferCount = 4;
        vkAllocateCommandBuffers(Parent.GetVkDevice(), &allocInfo, r.CommandBuffers.data());

        r.NextFreeCommandBuffer = 0;
    }
    CurrentFrameResourcesIndex = 0;
}

void CSubmissionTracker::Shutdown()
{
    vkDeviceWaitIdle(Parent.GetVkDevice());

    while (!JobQueue.empty())
        PopFrontJob(true);

    for (const auto& pair : TransientPools)
        for (const auto& pair2 : pair.second)
        {
            vkDestroyCommandPool(Parent.GetVkDevice(), pair2.second, nullptr);
        }
    TransientPools.clear();

    for (uint32_t i = 0; i < MaxFramesInFlight; i++)
    {
        CFrameResources& r = FrameResources[i];
        vkFreeCommandBuffers(Parent.GetVkDevice(), r.CommandPool,
                             static_cast<uint32_t>(r.CommandBuffers.size()),
                             r.CommandBuffers.data());
        vkDestroyCommandPool(Parent.GetVkDevice(), r.CommandPool, nullptr);
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

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = static_cast<uint32_t>(jobInfo.CmdBuffers.size());
    submitInfo.pCommandBuffers = jobInfo.CmdBuffers.data();
    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(jobInfo.WaitSemaphores.size());
    submitInfo.pWaitSemaphores = jobInfo.WaitSemaphores.data();
    submitInfo.pWaitDstStageMask = jobInfo.WaitStages.data();
    if (!jobInfo.SignalSemaphores.empty())
    {
        submitInfo.signalSemaphoreCount = static_cast<uint32_t>(jobInfo.SignalSemaphores.size());
        submitInfo.pSignalSemaphores = jobInfo.SignalSemaphores.data();
    }

    VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    vkCreateFence(Parent.GetVkDevice(), &fenceInfo, nullptr, &jobInfo.Fence);

    VkQueue q = Parent.GetVkQueue(jobInfo.QueueType);
    vkQueueSubmit(q, 1, &submitInfo, jobInfo.Fence);

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
    if (wait)
        vkWaitForFences(Parent.GetVkDevice(), 1, &waitJob.Fence, VK_TRUE, UINT64_MAX);

    if (waitJob.bIsFrameJob)
    {
        Parent.GetHugeConstantBuffer()->FreeBlock();
        FrameResources[waitJob.FrameResourceIndex].InFlightLock.Unlock();
        vkResetCommandPool(Parent.GetVkDevice(),
                           FrameResources[waitJob.FrameResourceIndex].CommandPool, 0);
        FrameResources[waitJob.FrameResourceIndex].NextFreeCommandBuffer = 0;
        FrameJobCount--;
    }

    if (waitJob.Kind == ECommandContextKind::Deferred)
    {
        // Need to reset the command buffers and return them to the context
        for (VkCommandBuffer b : waitJob.CmdBuffers)
            waitJob.DeferredContext->DoneWithCmdBuffer(b);
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

VkCommandPool CSubmissionTracker::GetTransientPool(EQueueType queueType)
{
    VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    poolInfo.flags =
        VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = Parent.GetQueueFamily(queueType);

    auto id = std::this_thread::get_id();
    auto iter = TransientPools.find(id);
    if (iter == TransientPools.end())
    {
        // Create a new pool
        VkCommandPool cmdPool;
        vkCreateCommandPool(Parent.GetVkDevice(), &poolInfo, nullptr, &cmdPool);
        TransientPools[id][queueType] = cmdPool;
        return cmdPool;
    }

    auto iter2 = iter->second.find(queueType);
    if (iter2 == iter->second.end())
    {
        VkCommandPool cmdPool;
        vkCreateCommandPool(Parent.GetVkDevice(), &poolInfo, nullptr, &cmdPool);
        TransientPools[id][queueType] = cmdPool;
        return cmdPool;
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
