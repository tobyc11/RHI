#include "ImageViewMetal.h"
#include "MtlHelpers.h"

namespace RHI
{

CImageViewMetal::CImageViewMetal(const CImageViewDesc& desc, CImage::Ref image)
    : Desc(desc)
    , SourceImage(image)
{
    auto* metalImage = static_cast<CImageMetal*>(image.get());

    if (auto* swapImg = dynamic_cast<CSwapChainImageMetal*>(metalImage))
    {
        bIsSwapChainProxy = true;
        TextureView = nil;
        return;
    }

    id<MTLTexture> srcTex = (id<MTLTexture>)metalImage->GetMTLTexture();
    MTLPixelFormat pf = MtlCast(desc.Format);
    MTLTextureType texType = MtlTextureType(desc.Type);

    NSRange mipRange = NSMakeRange(desc.Range.BaseMipLevel, desc.Range.LevelCount);
    NSRange sliceRange = NSMakeRange(desc.Range.BaseArrayLayer, desc.Range.LayerCount);

    TextureView = [srcTex newTextureViewWithPixelFormat:pf
                                           textureType:texType
                                                levels:mipRange
                                                slices:sliceRange];
}

CImageViewMetal::~CImageViewMetal() { }

id CImageViewMetal::GetMTLTexture() const
{
    if (bIsSwapChainProxy)
    {
        auto* swapImg = static_cast<CSwapChainImageMetal*>(SourceImage.get());
        return swapImg->GetMTLTexture();
    }
    return TextureView;
}

} /* namespace RHI */
