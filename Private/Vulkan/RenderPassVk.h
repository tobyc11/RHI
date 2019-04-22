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

    VkRenderPass GetHandle() const { return RenderPass; }
    const std::vector<VkAttachmentDescription>& GetAttachmentDesc() const { return AttachmentsVk; }
    const std::vector<CImageView::Ref>& GetAttachmentViews() const { return AttachmentViews; }
    VkRect2D GetArea() const { return Area; }

    // TODO: support multiple subpasses
    uint32_t SubpassColorAttachmentCount(uint32_t subpass) { return ColorAttachmentCount; }

    std::pair<VkFramebuffer, VkSemaphore> MakeFramebuffer();

private:
    CDeviceVk& Parent;
    VkRenderPass RenderPass;

    uint32_t ColorAttachmentCount = 0;
    std::vector<VkAttachmentDescription> AttachmentsVk;
    std::vector<CImageView::Ref> AttachmentViews; // Sole purpose is to hold images alive
    VkRect2D Area;
    uint32_t Layers;
};

} /* namespace RHI */
