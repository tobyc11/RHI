#pragma once
#include "ImageMetal.h"

namespace RHI
{

class CImageViewMetal : public CImageView
{
public:
    typedef std::shared_ptr<CImageViewMetal> Ref;

    CImageViewMetal(const CImageViewDesc& desc, CImage::Ref image);
    ~CImageViewMetal() override;

    id GetMTLTexture() const;
    CImageViewDesc GetDesc() const { return Desc; }

    bool IsSwapChainProxy() const { return bIsSwapChainProxy; }

private:
    CImageViewDesc Desc;
    CImage::Ref SourceImage;
    id TextureView;
    bool bIsSwapChainProxy = false;
};

} /* namespace RHI */
