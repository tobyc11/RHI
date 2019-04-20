#include "RenderPassVk.h"
#include "DeviceVk.h"
#include "ImageViewVk.h"
#include "SwapChainVk.h"
#include "VkHelpers.h"

namespace RHI
{

CRenderPassVk::CRenderPassVk(CDeviceVk& p, const CRenderPassDesc& desc)
    : Parent(p)
{
    VkRenderPassCreateInfo passInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };

    std::vector<VkImageView> swapChainViews;

    for (const auto& attachment : desc.Attachments)
    {
        auto viewImpl = std::static_pointer_cast<CImageViewVk>(attachment.ImageView);
        if (viewImpl->bIsSwapChainProxy)
        {
            auto swapChainImpl = std::static_pointer_cast<CSwapChainVk>(viewImpl->SwapChain.lock());
            swapChainViews = swapChainImpl->GetVkImageViews();
#ifdef _DEBUG
            SwapChainVersion = swapChainImpl->Version;
#endif

            bIsSwapChainProxy = true;
            SwapChain = viewImpl->SwapChain;
        }
        else
        {
            AttachmentViews.push_back(attachment.ImageView);
        }

        AttachmentsVk.push_back(VkAttachmentDescription());
        VkAttachmentDescription& r = AttachmentsVk.back();
        r.flags = 0;
        r.format = viewImpl->GetFormat();
        r.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: FIXME
        r.loadOp = static_cast<VkAttachmentLoadOp>(attachment.LoadOp);
        r.storeOp = static_cast<VkAttachmentStoreOp>(attachment.StoreOp);
        r.stencilLoadOp = static_cast<VkAttachmentLoadOp>(attachment.StencilLoadOp);
        r.stencilStoreOp = static_cast<VkAttachmentStoreOp>(attachment.StencilStoreOp);
        r.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // TODO: Layout undefined destroys content
        r.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        if (viewImpl->bIsSwapChainProxy)
            r.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        else
        {
            if (Any(viewImpl->GetImage()->GetUsageFlags(), EImageUsageFlags::Sampled))
                r.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            if (GetImageAspectFlags(r.format) & VK_IMAGE_ASPECT_DEPTH_BIT)
                r.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
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

        for (uint32_t inputIdx : subpass.InputAttachments)
        {
            allInputAttachments.push_back({ inputIdx, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
            subpassDescription.inputAttachmentCount++;
        }
        subpassDescription.pInputAttachments = allInputAttachments.data()
            + allInputAttachments.size() - subpassDescription.inputAttachmentCount;

        for (uint32_t colorIdx : subpass.ColorAttachments)
        {
            allColorAttachments.push_back({ colorIdx, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
            subpassDescription.colorAttachmentCount++;
        }
        subpassDescription.pColorAttachments = allColorAttachments.data()
            + allColorAttachments.size() - subpassDescription.colorAttachmentCount;

        if (subpass.DepthStencilAttachment != CSubpassDesc::None)
        {
            allDepthStencilAttachments.push_back(
                { subpass.DepthStencilAttachment,
                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });
            subpassDescription.pDepthStencilAttachment =
                allDepthStencilAttachments.data() + allDepthStencilAttachments.size() - 1;
        }

        subpassDescriptions.push_back(subpassDescription);
    }
    ColorAttachmentCount = subpassDescriptions[0].colorAttachmentCount;

    // Let's only handle the situation where there is only one subpass
    std::vector<VkSubpassDependency> dependency(1);
    dependency[0].dependencyFlags = 0;
    dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency[0].dstSubpass = 0;
    dependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency[0].srcAccessMask = 0;
    dependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    passInfo.attachmentCount = static_cast<uint32_t>(AttachmentsVk.size());
    passInfo.pAttachments = AttachmentsVk.data();
    passInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
    passInfo.pSubpasses = subpassDescriptions.data();
    passInfo.dependencyCount = static_cast<uint32_t>(dependency.size());
    passInfo.pDependencies = dependency.data();

    vkCreateRenderPass(Parent.GetVkDevice(), &passInfo, nullptr, &RenderPass);

    // Create the framebuffer, if not swapchain, just one is fine. Otherwise create as many as there
    // are swapchain images
    if (swapChainViews.empty())
    {
        // Not linking to a swap chain
        VkFramebufferCreateInfo fbInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        fbInfo.renderPass = RenderPass;
        std::vector<VkImageView> attachments;
        for (auto attachment : desc.Attachments)
        {
            auto viewImpl = std::static_pointer_cast<CImageViewVk>(attachment.ImageView);
            attachments.push_back(viewImpl->GetVkImageView());
        }
        fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        fbInfo.pAttachments = attachments.data();
        fbInfo.width = desc.Width;
        fbInfo.height = desc.Height;
        fbInfo.layers = desc.Layers;
        VkFramebuffer fb;
        vkCreateFramebuffer(Parent.GetVkDevice(), &fbInfo, nullptr, &fb);
        Framebuffer.push_back(fb);
    }
    for (VkImageView swapChainView : swapChainViews)
    {
        // Links to a swap chain, thus create multiple framebuffers
        VkFramebufferCreateInfo fbInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        fbInfo.renderPass = RenderPass;
        std::vector<VkImageView> attachments;
        for (auto attachment : desc.Attachments)
        {
            auto viewImpl = std::static_pointer_cast<CImageViewVk>(attachment.ImageView);
            if (viewImpl->bIsSwapChainProxy)
                attachments.push_back(swapChainView);
            else
                attachments.push_back(viewImpl->GetVkImageView());
        }
        fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        fbInfo.pAttachments = attachments.data();
        fbInfo.width = desc.Width;
        fbInfo.height = desc.Height;
        fbInfo.layers = desc.Layers;
        VkFramebuffer fb;
        vkCreateFramebuffer(Parent.GetVkDevice(), &fbInfo, nullptr, &fb);
        Framebuffer.push_back(fb);
    }

    Area.offset.x = Area.offset.y = 0;
    Area.extent.width = desc.Width;
    Area.extent.height = desc.Height;
}

CRenderPassVk::~CRenderPassVk()
{
    for (auto fb : Framebuffer)
    {
        vkDestroyFramebuffer(Parent.GetVkDevice(), fb, nullptr);
    }
    vkDestroyRenderPass(Parent.GetVkDevice(), RenderPass, nullptr);
    AttachmentViews.clear();
}

std::pair<VkFramebuffer, VkSemaphore> CRenderPassVk::GetNextFramebuffer()
{
    if (Framebuffer.size() == 1)
        return std::pair<VkFramebuffer, VkSemaphore>(Framebuffer[0], VK_NULL_HANDLE);

    auto swapChainImpl = std::static_pointer_cast<CSwapChainVk>(SwapChain.lock());
    if (swapChainImpl->AcquiredImages.empty())
        throw CRHIException("Did not acquire any image from the swap chain");
#ifdef _DEBUG
    if (SwapChainVersion != swapChainImpl->Version)
        throw CRHIException("Framebuffer (renderpass) outdated with respect to swap chain");
#endif
    return std::make_pair(Framebuffer[swapChainImpl->AcquiredImages.front().first],
                          swapChainImpl->AcquiredImages.front().second);
}

}
