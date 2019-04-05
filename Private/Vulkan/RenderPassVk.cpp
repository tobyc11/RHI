#include "RenderPassVk.h"
#include "DeviceVk.h"
#include "ImageViewVk.h"

namespace RHI
{

CRenderPassVk::CRenderPassVk(CDeviceVk& p, const CRenderPassDesc& desc)
    : Parent(p)
{
    VkRenderPassCreateInfo passInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };

    std::vector<VkAttachmentDescription> attachmentsVk;
    for (const auto& attachment : desc.Attachments)
    {
        attachmentsVk.push_back(VkAttachmentDescription());
        VkAttachmentDescription& r = attachmentsVk.back();
        r.flags = 0;
        r.format = static_cast<VkFormat>(attachment.Format);
        r.samples = static_cast<VkSampleCountFlagBits>(attachment.Samples);
        r.loadOp = static_cast<VkAttachmentLoadOp>(attachment.LoadOp);
        r.storeOp = static_cast<VkAttachmentStoreOp>(attachment.StoreOp);
        r.stencilLoadOp = static_cast<VkAttachmentLoadOp>(attachment.StencilLoadOp);
        r.stencilStoreOp = static_cast<VkAttachmentStoreOp>(attachment.StencilStoreOp);
        r.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        r.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    }

    std::vector<VkSubpassDescription> subpassDescriptions;
    std::vector<VkAttachmentReference> allInputAttachments;
    std::vector<VkAttachmentReference> allColorAttachments;
    std::vector<VkAttachmentReference> allResolveAttachments;
    std::vector<VkAttachmentReference> allDepthStencilAttachments;
    std::vector<uint32_t> allPreserveAttachments;

    for (const auto& subpass : desc.Subpasses)
    {
        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        subpassDescription.pInputAttachments =
            allInputAttachments.data() + allInputAttachments.size();
        for (uint32_t inputIdx : subpass.InputAttachments)
        {
            allInputAttachments.push_back({ inputIdx, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
            subpassDescription.inputAttachmentCount++;
        }

        subpassDescription.pColorAttachments =
            allColorAttachments.data() + allColorAttachments.size();
        for (uint32_t colorIdx : subpass.ColorAttachments)
        {
            allColorAttachments.push_back({ colorIdx, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
            subpassDescription.colorAttachmentCount++;
        }

        if (subpass.DepthStencilAttachment != CSubpassDesc::None)
        {
            subpassDescription.pDepthStencilAttachment =
                allDepthStencilAttachments.data() + allDepthStencilAttachments.size();
            allDepthStencilAttachments.push_back(
                { subpass.DepthStencilAttachment,
                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });
        }

        subpassDescriptions.push_back(subpassDescription);
    }
    ColorAttachmentCount = subpassDescriptions[0].colorAttachmentCount;

    // Let's only handle the situation where there is only one subpass
    // TODO:
    //std::vector<VkSubpassDependency> dependency(2);
    //dependency[0].dependencyFlags = 0;
    //dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    //dependency[0].dstSubpass = 0;
    //dependency[0].srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    //dependency[0].srcAccessMask = 0;
    //dependency[0].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    //dependency[0].dstAccessMask = 0;
    //dependency[1].dependencyFlags = 0;
    //dependency[1].srcSubpass = 0;
    //dependency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    //dependency[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    //dependency[1].srcAccessMask =
    //    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    //dependency[1].dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    //dependency[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    passInfo.attachmentCount = attachmentsVk.size();
    passInfo.pAttachments = attachmentsVk.data();
    passInfo.subpassCount = subpassDescriptions.size();
    passInfo.pSubpasses = subpassDescriptions.data();
    //passInfo.dependencyCount = dependency.size();
    //passInfo.pDependencies = dependency.data();

    vkCreateRenderPass(Parent.GetVkDevice(), &passInfo, nullptr, &RenderPass);
}

CRenderPassVk::~CRenderPassVk() { vkDestroyRenderPass(Parent.GetVkDevice(), RenderPass, nullptr); }

CFramebufferVk::CFramebufferVk(CDeviceVk& p, const CFramebufferDesc& desc)
    : Parent(p)
{
    VkFramebufferCreateInfo fbInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    fbInfo.renderPass = std::static_pointer_cast<CRenderPassVk>(desc.RenderPass)->RenderPass;
    std::vector<VkImageView> attachments;
    for (auto view : desc.Attachments)
        attachments.push_back(std::static_pointer_cast<CImageViewVk>(view)->ImageView);
    fbInfo.attachmentCount = attachments.size();
    fbInfo.pAttachments = attachments.data();
    fbInfo.width = desc.Width;
    fbInfo.height = desc.Height;
    fbInfo.layers = desc.Layers;

    vkCreateFramebuffer(Parent.GetVkDevice(), &fbInfo, nullptr, &Framebuffer);
}

CFramebufferVk::~CFramebufferVk()
{
    vkDestroyFramebuffer(Parent.GetVkDevice(), Framebuffer, nullptr);
}

}
