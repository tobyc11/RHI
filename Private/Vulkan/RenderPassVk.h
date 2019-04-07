#pragma once
#include "RenderPass.h"
#include "SwapChain.h"
#include "VkCommon.h"
#include <vector>

namespace RHI
{

class CRenderPassVk : public CRenderPass
{
public:
    CRenderPassVk(CDeviceVk& p, const CRenderPassDesc& desc);
    ~CRenderPassVk() override;

    const std::vector<VkAttachmentDescription>& GetAttachmentDesc() const { return AttachmentsVk; }

    // TODO: support multiple subpasses
    uint32_t SubpassColorAttachmentCount(uint32_t subpass) { return ColorAttachmentCount; }

    VkRect2D GetArea() const { return Area; }

    VkRenderPass RenderPass;
    std::vector<VkFramebuffer> Framebuffer;

    bool bIsSwapChainProxy;
    CSwapChain::WeakRef SwapChain;
    std::pair<VkFramebuffer, VkSemaphore> GetNextFramebuffer();

private:
    CDeviceVk& Parent;
    uint32_t ColorAttachmentCount = 0;
    std::vector<VkAttachmentDescription> AttachmentsVk;
    VkRect2D Area;
};

} /* namespace RHI */
