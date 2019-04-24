#pragma once
#include "RenderPass.h"
#include "VkCommon.h"
#include <SpinLock.h>
#include <memory>
#include <queue>

namespace RHI
{

enum EQueueType
{
    QT_TRANSFER,
    QT_COMPUTE,
    QT_GRAPHICS,
    NUM_QUEUE_TYPES
};

class CCommandBufferVk;

class CCommandPoolVk : public std::enable_shared_from_this<CCommandPoolVk>
{
public:
    typedef std::shared_ptr<CCommandPoolVk> Ref;

    CCommandPoolVk(CDeviceVk& p, EQueueType queueType, bool resetIndividualBuffer = true);
    ~CCommandPoolVk();

    CDeviceVk& GetParent() const { return Parent; }
    VkCommandPool GetHandle() const { return Handle; }
    std::unique_ptr<CCommandBufferVk> AllocateCommandBuffer(bool secondary = false);
    void ResetPool();

private:
    friend class CCommandBufferVk;

    CDeviceVk& Parent;
    VkCommandPool Handle;
    bool bResetIndividualBuffer;

    // If this pool doesn't support freeing individual buffers, used ones go into freed list first,
    // after reset pool, they go into reset list ready for reallocation
    tc::FSpinLock BufferListLock;
    std::queue<VkCommandBuffer> FreedPrimaryBuffers;
    std::queue<VkCommandBuffer> FreedSecondaryBuffers;
    bool bCanAllocateFromFreedList;
};

class CCommandBufferVk
{
public:
    CCommandBufferVk(CCommandPoolVk::Ref pool, bool secondary = false);
    CCommandBufferVk(CCommandPoolVk::Ref pool, VkCommandBuffer handle, bool secondary = false);
    ~CCommandBufferVk();

    VkCommandBuffer GetHandle() const { return Handle; }
    void BeginRecording(CRenderPass::Ref renderPass, uint32_t subpass);
    void EndRecording();

private:
    CCommandPoolVk::Ref CommandPool;
    VkCommandBuffer Handle;
    bool bIsSecondary;
};

}
