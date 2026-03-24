#include "SwapChainMetal.h"
#include "DeviceMetal.h"
#include "ImageViewMetal.h"
#include "MtlHelpers.h"

#import <AppKit/AppKit.h>

namespace RHI
{

CSwapChainMetal::CSwapChainMetal(CDeviceMetal& parent, const CPresentationSurfaceDesc& surfaceDesc,
                                 EFormat format)
    : Parent(parent)
    , Format(format)
    , CurrentDrawable(nil)
{
    NSView* view = (__bridge NSView*)surfaceDesc.MacOS.View;
    if (!view)
        throw CRHIRuntimeError("Null NSView in presentation surface descriptor");

    CAMetalLayer* metalLayer = [CAMetalLayer layer];
    metalLayer.device = (id<MTLDevice>)parent.GetMTLDevice();
    metalLayer.pixelFormat = MtlCast(format);
    metalLayer.framebufferOnly = YES;
    Layer = metalLayer;

    view.wantsLayer = YES;
    view.layer = metalLayer;

    CGSize size = view.bounds.size;
    CGFloat scale = view.window ? view.window.backingScaleFactor : 1.0;
    Width = static_cast<uint32_t>(size.width * scale);
    Height = static_cast<uint32_t>(size.height * scale);
    metalLayer.drawableSize = CGSizeMake(Width, Height);

    SwapChainImage = std::make_shared<CSwapChainImageMetal>(format, Width, Height);
}

CSwapChainMetal::~CSwapChainMetal() { }

void CSwapChainMetal::Resize(uint32_t width, uint32_t height)
{
    CAMetalLayer* metalLayer = (CAMetalLayer*)Layer;
    if (width == UINT32_MAX || height == UINT32_MAX)
    {
        CGSize size = metalLayer.bounds.size;
        CGFloat scale = metalLayer.contentsScale;
        width = static_cast<uint32_t>(size.width * scale);
        height = static_cast<uint32_t>(size.height * scale);
    }

    if (width == 0 || height == 0)
        return;

    Width = width;
    Height = height;
    metalLayer.drawableSize = CGSizeMake(width, height);
    SwapChainImage->SetSize(width, height);
}

void CSwapChainMetal::GetSize(uint32_t& width, uint32_t& height) const
{
    width = Width;
    height = Height;
}

CImage::Ref CSwapChainMetal::GetImage() { return SwapChainImage; }

bool CSwapChainMetal::AcquireNextImage()
{
    CAMetalLayer* metalLayer = (CAMetalLayer*)Layer;
    CurrentDrawable = [metalLayer nextDrawable];
    if (!CurrentDrawable)
        return false;

    id<CAMetalDrawable> drawable = (id<CAMetalDrawable>)CurrentDrawable;
    SwapChainImage->SetTexture(drawable.texture);
    SwapChainImage->SetSize(static_cast<uint32_t>(drawable.texture.width),
                            static_cast<uint32_t>(drawable.texture.height));
    return true;
}

void CSwapChainMetal::Present(const CSwapChainPresentInfo& info)
{
    if (!CurrentDrawable)
        return;

    id<MTLCommandBuffer> cmdBuf = [(id<MTLCommandQueue>)Parent.GetDefaultQueue() commandBuffer];
    [cmdBuf presentDrawable:(id<CAMetalDrawable>)CurrentDrawable];
    [cmdBuf commit];

    CurrentDrawable = nil;
}

} /* namespace RHI */
