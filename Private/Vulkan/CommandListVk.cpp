#include "CommandListVk.h"
#include "CommandContextVk.h"
#include "CommandQueueVk.h"

namespace RHI
{

VkSubmitInfo CCommandListSection::MakeSubmitInfo(std::vector<VkCommandBuffer>& stagingArray) const
{
    stagingArray.push_back(PreCmdBuffer->GetHandle());
    stagingArray.push_back(CmdBuffer->GetHandle());

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.waitSemaphoreCount = WaitSemaphores.size();
    submitInfo.pWaitSemaphores = WaitSemaphores.data();
    submitInfo.pWaitDstStageMask = WaitStages.data();
    submitInfo.commandBufferCount = 2;
    submitInfo.pCommandBuffers = stagingArray.data() + stagingArray.size() - 2;
    submitInfo.signalSemaphoreCount = SignalSemaphores.size();
    submitInfo.pSignalSemaphores = SignalSemaphores.data();
    return submitInfo;
}

CCommandListVk::CCommandListVk(CCommandQueueVk& p)
    : Parent(p)
{
}

void CCommandListVk::Enqueue()
{
    if (bIsQueued)
        return;

    Parent.EnqueueCommandList(std::static_pointer_cast<CCommandListVk>(shared_from_this()));
    bIsQueued = true;
}

void CCommandListVk::Commit()
{
    if (bIsCommitted)
        return;

    if (!bIsQueued)
        Enqueue();

    if (bIsContextActive)
        throw CRHIRuntimeError("Can't commit when the list is still being recorded");

    bIsCommitted = true;
}

ICopyContext::Ref CCommandListVk::CreateCopyContext()
{
    return std::make_shared<CCommandContextVk>(
        std::static_pointer_cast<CCommandListVk>(shared_from_this()));
}

IComputeContext::Ref CCommandListVk::CreateComputeContext()
{
    return std::make_shared<CCommandContextVk>(
        std::static_pointer_cast<CCommandListVk>(shared_from_this()));
}

IParallelRenderContext::Ref
CCommandListVk::CreateParallelRenderContext(CRenderPass::Ref renderPass,
                                            const std::vector<CClearValue>& clearValues)
{
    return std::make_shared<CRenderPassContextVk>(
        std::static_pointer_cast<CCommandListVk>(shared_from_this()), renderPass, clearValues);
}

void CCommandListVk::MakeSubmitInfos(std::vector<VkSubmitInfo>& submitInfos,
                                     std::vector<VkCommandBuffer>& stagingArray)
{
    if (!Sections.empty())
    {
        assert(Sections[0].PreCmdBuffer == nullptr);
        Sections[0].PreCmdBuffer = GetQueue().GetCmdBufferAllocator().Allocate();
        Sections[0].PreCmdBuffer->BeginRecording(VK_NULL_HANDLE, 0);
        Sections[0].AccessTracker.DeployAllBarriers(Sections[0].PreCmdBuffer->GetHandle());
        Sections[0].AccessTracker.Clear();
        Sections[0].PreCmdBuffer->EndRecording();
    }

    for (const auto& iter : Sections)
        submitInfos.emplace_back(iter.MakeSubmitInfo(stagingArray));
}

void CCommandListVk::ReleaseAllResources() { Sections.clear(); }

}
