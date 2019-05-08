#pragma once
#include "AccessTracker.h"
#include "CommandBufferVk.h"
#include "VkCommon.h"
#include "CopyContext.h"
#include "ComputeContext.h"
#include "RenderContext.h"
#include <memory>
#include <vector>

namespace RHI
{

// A command list of made up from multiple sections (copy pass, compute pass, etc)
struct CCommandListSection
{
    std::vector<VkSemaphore> WaitSemaphores;
    std::vector<VkPipelineStageFlags> WaitStages;

    std::unique_ptr<CCommandBufferVk> PreCmdBuffer;
    std::unique_ptr<CCommandBufferVk> CmdBuffer;
    std::vector<std::unique_ptr<CCommandBufferVk>> SecondaryBuffers;

    std::vector<VkSemaphore> SignalSemaphores;

    CAccessTracker AccessTracker;

    VkSubmitInfo MakeSubmitInfo(std::vector<VkCommandBuffer>& stagingArray) const;
};

class CCommandListVk : public CCommandList
{
    friend class CCommandContextVk;
    friend class CRenderPassContextVk;

public:
    typedef std::shared_ptr<CCommandListVk> Ref;

    explicit CCommandListVk(CCommandQueueVk& p);

    CCommandQueueVk& GetQueue() const { return Parent; }
    bool IsQueued() const { return bIsQueued; }
    bool IsCommitted() const { return bIsCommitted; }

    void Enqueue() override;
    void Commit() override;

    ICopyContext::Ref CreateCopyContext() override;
    IComputeContext::Ref CreateComputeContext() override;
    IParallelRenderContext::Ref CreateParallelRenderContext(CRenderPass::Ref renderPass,
                                                            const std::vector<CClearValue>& clearValues) override;

    void MakeSubmitInfos(std::vector<VkSubmitInfo>& submitInfos, std::vector<VkCommandBuffer>& stagingArray);
    void ReleaseAllResources();

private:
    // Not holding a reference to prevent circular reference
    CCommandQueueVk& Parent;
    // Whether this command list is enqueued
    bool bIsQueued = false;
    // Whether this command list is ready for submission
    bool bIsCommitted = false;

    // The context has access to all the temporary states
    // NOTE: Sections[0].AccessTracker tracks the entire command list
    // NOTE: Each of these "sections" become a VkSubmitInfo
    std::vector<CCommandListSection> Sections;
    // Whether there is a context currently recording into this
    bool bIsContextActive = false;
};

}
