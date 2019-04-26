#include "ImageViewVk.h"
#include "DeviceVk.h"
#include "SwapChainVk.h"
#include "VkHelpers.h"

namespace RHI
{

static VkImageViewType ImageToViewType(VkImageType type)
{
    switch (type)
    {
    case VK_IMAGE_TYPE_1D:
        return VK_IMAGE_VIEW_TYPE_1D;
        break;
    case VK_IMAGE_TYPE_2D:
        return VK_IMAGE_VIEW_TYPE_2D;
        break;
    case VK_IMAGE_TYPE_3D:
        return VK_IMAGE_VIEW_TYPE_3D;
    default:
        throw CRHIRuntimeError("Unable to create image view: image type corruption");
        break;
    }
}

static VkImageViewType Convert(EImageViewType type)
{
    switch (type)
    {
    case RHI::EImageViewType::View1D:
        return VK_IMAGE_VIEW_TYPE_1D;
    case RHI::EImageViewType::View2D:
        return VK_IMAGE_VIEW_TYPE_2D;
    case RHI::EImageViewType::View3D:
        return VK_IMAGE_VIEW_TYPE_3D;
    case RHI::EImageViewType::Cube:
        return VK_IMAGE_VIEW_TYPE_CUBE;
    case RHI::EImageViewType::View1DArray:
        return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
    case RHI::EImageViewType::View2DArray:
        return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    case RHI::EImageViewType::CubeArray:
        return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
    default:
        throw CRHIRuntimeError("Unable to create image view: image type corruption");
    }
}

static void Convert(VkImageSubresourceRange& dst, const CImageSubresourceRange& src)
{
    dst.baseArrayLayer = src.BaseArrayLayer;
    dst.baseMipLevel = src.BaseMipLevel;
    dst.layerCount = src.LayerCount;
    dst.levelCount = src.LevelCount;
}

CImageViewVk::CImageViewVk(CDeviceVk& p, const CImageViewDesc& desc, CImageVk::Ref image)
    : Parent(p)
    , Image(image)
    , bIsSwapChainProxy(false)
    , ViewCreateInfo()
    , ImageView(VK_NULL_HANDLE)
{
    auto* imgVk = static_cast<CImageVk*>(image.get());
    if (imgVk->IsSwapChainProxy())
    {
        // The image passed in is a proxy image
        auto* swapImageVk = static_cast<CSwapChainImageVk*>(imgVk);
        bIsSwapChainProxy = true;
        SwapChain = swapImageVk->GetSwapChain();

        // Set resource range in view create info to match the swap chain
        ViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        ViewCreateInfo.subresourceRange.baseMipLevel = 0;
        ViewCreateInfo.subresourceRange.layerCount = 1;
        ViewCreateInfo.subresourceRange.levelCount = 1;
        return;
    }

    ViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    ViewCreateInfo.image = imgVk->GetVkImage();
    ViewCreateInfo.viewType = Convert(desc.Type);
    ViewCreateInfo.format = static_cast<VkFormat>(desc.Format);
    Convert(ViewCreateInfo.subresourceRange, desc.Range);
    ViewCreateInfo.subresourceRange.aspectMask = GetImageAspectFlags(ViewCreateInfo.format);
    // Only if aspect flags are set and we are dealing with a DepthStencil format
    if (ViewCreateInfo.subresourceRange.aspectMask != VK_IMAGE_ASPECT_COLOR_BIT)
    {
        // We are handling a depth stencil texture
        ViewCreateInfo.subresourceRange.aspectMask = 0;
        if (Any(desc.DepthStencilAspect, EDepthStencilAspectFlags::Depth))
            ViewCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
        if (Any(desc.DepthStencilAspect, EDepthStencilAspectFlags::Stencil))
            ViewCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    if (vkCreateImageView(Parent.GetVkDevice(), &ViewCreateInfo, nullptr, &ImageView) != VK_SUCCESS)
        throw CRHIRuntimeError("Unable to create image view");
}

CImageViewVk::~CImageViewVk()
{
    if (bIsSwapChainProxy)
        return;
    vkDestroyImageView(Parent.GetVkDevice(), ImageView, nullptr);
}

VkImageView CImageViewVk::GetVkImageView() const
{
    if (bIsSwapChainProxy)
        throw CRHIException("Proxy image view does not have actual view");
    return ImageView;
}

CImageVk::Ref CImageViewVk::GetImage() const { return Image; }

VkFormat CImageViewVk::GetFormat() const
{
    if (bIsSwapChainProxy)
        return std::static_pointer_cast<CSwapChainVk>(SwapChain.lock())->GetChosenFormat();
    return ViewCreateInfo.format;
}

CImageSubresourceRange CImageViewVk::GetResourceRange() const
{
    CImageSubresourceRange result;
    result.BaseArrayLayer = ViewCreateInfo.subresourceRange.baseArrayLayer;
    result.BaseMipLevel = ViewCreateInfo.subresourceRange.baseMipLevel;
    result.LayerCount = ViewCreateInfo.subresourceRange.layerCount;
    result.LevelCount = ViewCreateInfo.subresourceRange.levelCount;
    return result;
}

}
