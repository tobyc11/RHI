#pragma once
#include "MtlCommon.h"
#include "PresentationSurfaceDesc.h"
#include "SwapChain.h"
#include "ImageMetal.h"

namespace RHI
{

class CSwapChainMetal : public CSwapChain
{
public:
    typedef std::shared_ptr<CSwapChainMetal> Ref;

    CSwapChainMetal(CDeviceMetal& parent, const CPresentationSurfaceDesc& surfaceDesc,
                    EFormat format);
    ~CSwapChainMetal() override;

    void Resize(uint32_t width, uint32_t height) override;
    void GetSize(uint32_t& width, uint32_t& height) const override;

    CImage::Ref GetImage() override;

    bool AcquireNextImage() override;
    void Present(const CSwapChainPresentInfo& info) override;

private:
    CDeviceMetal& Parent;
    id Layer;       // CAMetalLayer*
    id CurrentDrawable; // id<CAMetalDrawable>
    std::shared_ptr<CSwapChainImageMetal> SwapChainImage;
    EFormat Format;
    uint32_t Width = 0;
    uint32_t Height = 0;
};

} /* namespace RHI */
