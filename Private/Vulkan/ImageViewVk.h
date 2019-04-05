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

    VkImageView ImageView;

private:
    CDeviceVk& Parent;
    CImage::Ref Image;
};

} /* namespace RHI */
