#include "CommandBufferVk.h"
#include "DeviceVk.h"
#include "RenderPassVk.h"

namespace RHI
{

CCommandPoolVk::CCommandPoolVk(CDeviceVk& p, EQueueType queueType, bool resetIndividualBuffer)
    : Parent(p)
    , bResetIndividualBuffer(resetIndividualBuffer)
    , bCanAllocateFromFreedList(false)
{
    VkCommandPoolCreateInfo ci = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    ci.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    if (resetIndividualBuffer)
        ci.flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ci.queueFamilyIndex = Parent.GetQueueFamily(queueType);
    VK(vkCreateCommandPool(Parent.GetVkDevice(), &ci, nullptr, &Handle));
}

CCommandPoolVk::~CCommandPoolVk() { vkDestroyCommandPool(Parent.GetVkDevice(), Handle, nullptr); }

std::unique_ptr<CCommandBufferVk> CCommandPoolVk::AllocateCommandBuffer(bool secondary)
{
    if (bResetIndividualBuffer || !bCanAllocateFromFreedList)
        return std::make_unique<CCommandBufferVk>(shared_from_this(), secondary);

    std::unique_lock<tc::FSpinLock> lk(BufferListLock);
    if (secondary)
    {
        if (FreedSecondaryBuffers.empty())
            return std::make_unique<CCommandBufferVk>(shared_from_this(), secondary);
        VkCommandBuffer buffer = FreedSecondaryBuffers.front();
        FreedSecondaryBuffers.pop();
        return std::make_unique<CCommandBufferVk>(shared_from_this(), buffer, secondary);
    }
    else
    {
        if (FreedPrimaryBuffers.empty())
            return std::make_unique<CCommandBufferVk>(shared_from_this(), secondary);
        VkCommandBuffer buffer = FreedPrimaryBuffers.front();
        FreedPrimaryBuffers.pop();
        return std::make_unique<CCommandBufferVk>(shared_from_this(), buffer, secondary);
    }
}

void CCommandPoolVk::ResetPool()
{
    VK(vkResetCommandPool(Parent.GetVkDevice(), Handle, 0));
    bCanAllocateFromFreedList = true;
}

CCommandBufferVk::CCommandBufferVk(CCommandPoolVk::Ref pool, bool secondary)
    : CommandPool(pool)
    , bIsSecondary(secondary)
{
    VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool = pool->GetHandle();
    if (secondary)
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    else
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VK(vkAllocateCommandBuffers(CommandPool->GetParent().GetVkDevice(), &allocInfo, &Handle));
}

CCommandBufferVk::CCommandBufferVk(CCommandPoolVk::Ref pool, VkCommandBuffer handle, bool secondary)
    : CommandPool(pool)
    , Handle(handle)
    , bIsSecondary(secondary)
{
}

CCommandBufferVk::~CCommandBufferVk()
{
    if (CommandPool->bResetIndividualBuffer)
        vkFreeCommandBuffers(CommandPool->GetParent().GetVkDevice(), CommandPool->GetHandle(), 1,
                             &Handle);
    else
    {
        std::unique_lock<tc::FSpinLock> lk(CommandPool->BufferListLock);
        if (bIsSecondary)
            CommandPool->FreedSecondaryBuffers.push(Handle);
        else
            CommandPool->FreedPrimaryBuffers.push(Handle);
        CommandPool->bCanAllocateFromFreedList = false;
    }

    if (BufferAllocator)
        BufferAllocator->UnlockPool(AllocatorPoolIndex);
    BufferAllocator = nullptr;
}

void CCommandBufferVk::BeginRecording(CRenderPass::Ref renderPass, uint32_t subpass)
{
    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    VkCommandBufferInheritanceInfo inheritInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO
    };

    if (renderPass)
    {
        if (!bIsSecondary)
            throw CRHIException("Render pass command buffer must be secondary");

        auto rpImpl = std::static_pointer_cast<CRenderPassVk>(renderPass);
        inheritInfo.renderPass = rpImpl->GetHandle();
        inheritInfo.subpass = subpass;
        inheritInfo.framebuffer = VK_NULL_HANDLE;
        inheritInfo.occlusionQueryEnable = VK_FALSE;
        inheritInfo.queryFlags = 0;
        inheritInfo.pipelineStatistics = 0;
        beginInfo.pInheritanceInfo = &inheritInfo;

        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }

    beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK(vkBeginCommandBuffer(Handle, &beginInfo));
}

void CCommandBufferVk::EndRecording()
{
    VkResult result = vkEndCommandBuffer(Handle);
    if (result != VK_SUCCESS)
        throw CRHIRuntimeError("Could not end command buffer");

    if (BufferAllocator)
        BufferAllocator->UnlockPool(AllocatorPoolIndex);
    BufferAllocator = nullptr;
}

CCommandBufferAllocatorVk::CCommandBufferAllocatorVk(CDeviceVk& deviceVk, EQueueType queueType)
{
    for (size_t i = 0; i < kNumPools; i++)
    {
        auto pool = std::make_shared<CCommandPoolVk>(deviceVk, queueType, true);
        Pools.emplace_back(std::move(pool));
        AvailablePools.push(i);
    }
}

std::unique_ptr<CCommandBufferVk> CCommandBufferAllocatorVk::Allocate(bool secondary)
{
    Mutex.Lock();
    while (AvailablePools.empty())
    {
        Mutex.Unlock();
        Mutex.Lock();
    }
    size_t poolIndex = AvailablePools.top();
    AvailablePools.pop();
    Mutex.Unlock();

    auto cmdBuffer = Pools[poolIndex]->AllocateCommandBuffer(secondary);
    cmdBuffer->BufferAllocator = this;
    cmdBuffer->AllocatorPoolIndex = poolIndex;
    return std::move(cmdBuffer);
}

void CCommandBufferAllocatorVk::UnlockPool(size_t index)
{
    std::lock_guard<tc::FSpinLock> lk(Mutex);
    AvailablePools.push(index);
}

}
