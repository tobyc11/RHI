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

static void Convert(VkImageSubresourceRange& dst, const CImageSubresourceRange& src)
{
    dst.baseArrayLayer = src.BaseArrayLayer;
    dst.baseMipLevel = src.BaseMipLevel;
    dst.layerCount = src.LayerCount;
    dst.levelCount = src.LevelCount;
}

CImageViewVk::CImageViewVk(CDeviceVk& p, const CImageViewDesc& desc, CImage::Ref image)
    : Parent(p)
    , Image(image)
    , bIsSwapChainProxy(false)
    , ViewCreateInfo()
    , ImageView(VK_NULL_HANDLE)
{
    auto* imgVk = static_cast<CImageVk*>(image.get());
    if (imgVk->bIsSwapChainProxy)
    {
        // The image passed in is a proxy image
        bIsSwapChainProxy = true;
        SwapChain = imgVk->SwapChain;
        return;
    }

    ViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    ViewCreateInfo.image = imgVk->GetVkImage();
    ViewCreateInfo.viewType = ImageToViewType(imgVk->GetCreateInfo().imageType);
    ViewCreateInfo.format = static_cast<VkFormat>(desc.Format);
    Convert(ViewCreateInfo.subresourceRange, desc.Range);
    ViewCreateInfo.subresourceRange.aspectMask = GetImageAspectFlags(ViewCreateInfo.format);

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

VkFormat CImageViewVk::GetFormat() const
{
    if (bIsSwapChainProxy)
        return std::static_pointer_cast<CSwapChainVk>(SwapChain.lock())->GetChosenFormat().format;
    return ViewCreateInfo.format;
}

}
