#pragma once
#include "CommandBufferVk.h"
#include "CommandQueue.h"
#include "CommandListVk.h"
#include "VkCommon.h"
#include <SpinLock.h>
#include <array>
#include <vector>

namespace RHI
{

class CCommandQueueVk : public CCommandQueue
{
public:
    typedef std::shared_ptr<CCommandQueueVk> Ref;

    CCommandQueueVk(CDeviceVk& p, EQueueType queueType, VkQueue handle);
    ~CCommandQueueVk() override;

    CDeviceVk& GetDevice() const { return Parent; }
    EQueueType GetType() const { return Type; }
    VkQueue GetHandle() const { return Handle; }
    CCommandBufferAllocatorVk& GetCmdBufferAllocator() { return CmdBufferAllocator; }

    CCommandList::Ref CreateCommandList() override;

    void Flush() override;
    void Finish() override;

    // Reserve a spot for the command list in this queue
    void EnqueueCommandList(CCommandListVk::Ref cmdList);

    // Submit all committed command lists
    void Submit(bool setFence = false);
    // Submit and advance frame index
    void SubmitFrame();

private:
    CDeviceVk& Parent;
    EQueueType Type;
    VkQueue Handle = VK_NULL_HANDLE;
    CCommandBufferAllocatorVk CmdBufferAllocator;

    std::mutex Mutex;

    std::vector<CCommandListVk::Ref> QueuedLists;

    static const uint32_t FrameIndexCount = 3;
    struct CFrameResources
    {
        CDeviceVk& DeviceVk;
        VkFence Fence = VK_NULL_HANDLE;

        std::vector<CCommandListVk::Ref> ListsInFlight;
        std::vector<std::function<void(CDeviceVk&)>> PostFrameCleanup;

        CFrameResources(CDeviceVk& deviceVk);
        ~CFrameResources();

        void Reset();
    };
    std::array<CFrameResources, FrameIndexCount> FrameResources;
    uint32_t CurrFrameIndex = 0;
};

}
