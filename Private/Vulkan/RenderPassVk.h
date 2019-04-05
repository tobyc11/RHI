#pragma once
#include "RenderPass.h"
#include "VkCommon.h"

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

    VkRenderPass RenderPass;

private:
    CDeviceVk& Parent;
    uint32_t ColorAttachmentCount = 0;
    std::vector<VkAttachmentDescription> AttachmentsVk;
};

class CFramebufferVk : public CFramebuffer
{
public:
    CFramebufferVk(CDeviceVk& p, const CFramebufferDesc& desc);
    ~CFramebufferVk() override;

	const VkRect2D& GetArea() const { return Area; }

    VkFramebuffer Framebuffer;

private:
    CDeviceVk& Parent;
    VkRect2D Area;
};

} /* namespace RHI */
