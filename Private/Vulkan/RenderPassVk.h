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

    // TODO: support multiple subpasses
    uint32_t SubpassColorAttachmentCount(uint32_t subpass) { return ColorAttachmentCount; }

    VkRenderPass RenderPass;

private:
    CDeviceVk& Parent;
    uint32_t ColorAttachmentCount = 0;
};

class CFramebufferVk : public CFramebuffer
{
public:
    CFramebufferVk(CDeviceVk& p, const CFramebufferDesc& desc);
    ~CFramebufferVk() override;

    VkFramebuffer Framebuffer;

private:
    CDeviceVk& Parent;
};

} /* namespace RHI */
