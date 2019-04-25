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

    for (const auto& attachment : desc.Attachments)
    {
        auto viewImpl = std::static_pointer_cast<CImageViewVk>(attachment.ImageView);

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
            if (GetImageAspectFlags(r.format) & VK_IMAGE_ASPECT_DEPTH_BIT)
            {
                r.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                if (Any(viewImpl->GetImage()->GetUsageFlags(), EImageUsageFlags::Sampled))
                    r.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            }
            else if (Any(viewImpl->GetImage()->GetUsageFlags(), EImageUsageFlags::Sampled))
                r.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        AttachmentViews.push_back(attachment.ImageView);
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

    Area.offset.x = Area.offset.y = 0;
    Area.extent.width = desc.Width;
    Area.extent.height = desc.Height;
    Layers = desc.Layers;
}

CRenderPassVk::~CRenderPassVk()
{
    vkDestroyRenderPass(Parent.GetVkDevice(), RenderPass, nullptr);
    AttachmentViews.clear();
}

void CRenderPassVk::Resize(uint32_t width, uint32_t height)
{
    Area.extent.width = width;
    Area.extent.height = height;
}

std::pair<VkFramebuffer, VkSemaphore> CRenderPassVk::MakeFramebuffer()
{
    VkSemaphore waitSemaphore = VK_NULL_HANDLE;

    VkFramebufferCreateInfo fbInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    fbInfo.renderPass = RenderPass;
    std::vector<VkImageView> attachments;
    for (auto view : AttachmentViews)
    {
        auto viewImpl = std::static_pointer_cast<CImageViewVk>(view);
        if (viewImpl->bIsSwapChainProxy)
        {
            assert(!waitSemaphore);
            auto swapChainImpl = std::static_pointer_cast<CSwapChainVk>(viewImpl->SwapChain.lock());
            if (swapChainImpl->AcquiredImages.empty())
                throw CRHIException("Did not acquire any image from the swap chain");
            attachments.push_back(
                swapChainImpl->GetVkImageViews()[swapChainImpl->AcquiredImages.front().first]);
            waitSemaphore = swapChainImpl->AcquiredImages.front().second;
        }
        else
            attachments.push_back(viewImpl->GetVkImageView());
    }
    fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    fbInfo.pAttachments = attachments.data();
    fbInfo.width = Area.extent.width;
    fbInfo.height = Area.extent.height;
    fbInfo.layers = Layers;
    VkFramebuffer fb;
    vkCreateFramebuffer(Parent.GetVkDevice(), &fbInfo, nullptr, &fb);
    return std::make_pair(fb, waitSemaphore);
}

void CRenderPassVk::UpdateImageInitialAccess(CAccessTracker& tracker)
{
    for (size_t i = 0; i < AttachmentsVk.size(); i++)
    {
        auto viewImpl = std::static_pointer_cast<CImageViewVk>(AttachmentViews[i]);
        CImageVk* image = viewImpl->GetImage().get();
        CImageSubresourceRange imageRange = viewImpl->GetResourceRange();
        VkImageLayout layout = AttachmentsVk[i].initialLayout;
        CAccessRecord record {};
        if (layout == VK_IMAGE_LAYOUT_UNDEFINED)
        {
            tracker.TransitionImage(VK_NULL_HANDLE, image, imageRange, 0,
                                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, layout);
        }
        else if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            tracker.TransitionImage(VK_NULL_HANDLE, image, imageRange, 0,
                                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, layout);
        }
        else if (layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            tracker.TransitionImage(VK_NULL_HANDLE, image, imageRange, 0,
                                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, layout);
        }
        else if (layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
        {
            tracker.TransitionImage(VK_NULL_HANDLE, image, imageRange, 0,
                                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, layout);
        }
        else
        {
            throw CRHIException("Expecting the wrong image layout");
        }
    }
}

void CRenderPassVk::UpdateImageFinalAccess(CAccessTracker& tracker)
{
    for (size_t i = 0; i < AttachmentsVk.size(); i++)
    {
        auto viewImpl = std::static_pointer_cast<CImageViewVk>(AttachmentViews[i]);
        CImageVk* image = viewImpl->GetImage().get();
        CImageSubresourceRange imageRange = viewImpl->GetResourceRange();
        VkImageLayout layout = AttachmentsVk[i].finalLayout;
        tracker.TransitionImage(VK_NULL_HANDLE, image, imageRange, 0,
                                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, layout);
    }
}

}
