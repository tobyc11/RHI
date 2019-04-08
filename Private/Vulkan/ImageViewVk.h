#pragma once
#include "ImageVk.h"
#include "VkCommon.h"

namespace RHI
{

class CImageViewVk : public CImageView
{
public:
    typedef std::shared_ptr<CImageViewVk> Ref;

    CImageViewVk(CDeviceVk& p, const CImageViewDesc& desc, CImageVk::Ref image);
    ~CImageViewVk() override;

    VkImageView GetVkImageView() const;
    CImageVk::Ref GetImage() const;

    VkFormat GetFormat() const;

    // This object could be a mere proxy for a swapchain, and does not hold any real image view
    bool bIsSwapChainProxy;
    CSwapChain::WeakRef SwapChain;

private:
    CDeviceVk& Parent;
    CImageVk::Ref Image;
    VkImageViewCreateInfo ViewCreateInfo;
    VkImageView ImageView;
};

} /* namespace RHI */
