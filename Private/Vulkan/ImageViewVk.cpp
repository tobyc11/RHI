#include "ImageViewVk.h"
#include "DeviceVk.h"
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
{
    auto* imgVk = static_cast<CImageVk*>(image.get());

    VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewInfo.image = imgVk->Image;
    viewInfo.viewType = ImageToViewType(imgVk->GetCreateInfo().imageType);
    viewInfo.format = static_cast<VkFormat>(desc.Format);
    Convert(viewInfo.subresourceRange, desc.Range);
    viewInfo.subresourceRange.aspectMask = GetImageAspectFlags(viewInfo.format);

    if (vkCreateImageView(Parent.GetVkDevice(), &viewInfo, nullptr, &ImageView) != VK_SUCCESS)
        throw CRHIRuntimeError("Unable to create image view");
}

CImageViewVk::~CImageViewVk() { vkDestroyImageView(Parent.GetVkDevice(), ImageView, nullptr); }

}
