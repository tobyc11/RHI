#pragma once
#include "ImageVk.h"
#include "VkCommon.h"

namespace RHI
{

class CImageViewVk : public CImageView
{
public:
    CImageViewVk(CDeviceVk& p, const CImageViewDesc& desc, CImage::Ref image);
    ~CImageViewVk() override;

    VkImageView GetVkImageView() const;

    VkFormat GetFormat() const;

    // This object could be a mere proxy for a swapchain, and does not hold any real image view
    bool bIsSwapChainProxy;
    CSwapChain::WeakRef SwapChain;

private:
    CDeviceVk& Parent;
    CImage::Ref Image;
    VkImageViewCreateInfo ViewCreateInfo;
    VkImageView ImageView;
};

} /* namespace RHI */
