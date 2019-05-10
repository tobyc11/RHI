#pragma once
#include "PipelineStateDesc.h"
#include "RHIChooseImpl.h"
#include "Resources.h"
#include "Sampler.h"
#include <memory>

namespace RHI
{

struct CBufferCopy
{
    size_t SrcOffset;
    size_t DstOffset;
    size_t Size;
};

struct CImageSubresourceLayers
{
    uint32_t MipLevel;
    uint32_t BaseArrayLayer;
    uint32_t LayerCount;
};

struct CImageCopy
{
    CImageSubresourceLayers SrcSubresource;
    COffset3D SrcOffset;
    CImageSubresourceLayers DstSubresource;
    COffset3D DstOffset;
    CExtent3D Extent;
};

struct CBufferImageCopy
{
    size_t BufferOffset;
    uint32_t BufferRowLength;
    uint32_t BufferImageHeight;
    CImageSubresourceLayers ImageSubresource;
    COffset3D ImageOffset;
    CExtent3D ImageExtent;
};

struct CImageBlit
{
    CImageSubresourceLayers SrcSubresource;
    COffset3D SrcOffsets[2];
    CImageSubresourceLayers DstSubresource;
    COffset3D DstOffsets[2];
};

struct CImageResolve
{
    CImageSubresourceLayers SrcSubresource;
    COffset3D SrcOffset;
    CImageSubresourceLayers DstSubresource;
    COffset3D DstOffset;
    CExtent3D Extent;
};

struct CClearDepthStencilValue
{
    float Depth;
    uint32_t Stencil;
};

struct CClearValue
{
    union {
        union {
            float ColorFloat32[4];
            int32_t ColorInt32[4];
            uint32_t ColorUInt32[4];
        };
        CClearDepthStencilValue DepthStencilValue;
    };

    CClearValue(float r, float g, float b, float a)
    {
        ColorFloat32[0] = r;
        ColorFloat32[1] = g;
        ColorFloat32[2] = b;
        ColorFloat32[3] = a;
    }

    CClearValue(float d, uint32_t s)
        : DepthStencilValue { d, s }
    {
    }
};

class ICopyContext
{
public:
    typedef std::shared_ptr<ICopyContext> Ref;

    virtual ~ICopyContext() = default;

    virtual void ClearImage(CImage& image, const CClearValue& clearValue,
                            const CImageSubresourceRange& range) = 0;

    virtual void CopyBuffer(CBuffer& src, CBuffer& dst,
                            const std::vector<CBufferCopy>& regions) = 0;
    virtual void CopyImage(CImage& src, CImage& dst, const std::vector<CImageCopy>& regions) = 0;
    virtual void CopyBufferToImage(CBuffer& src, CImage& dst,
                                   const std::vector<CBufferImageCopy>& regions) = 0;
    virtual void CopyImageToBuffer(CImage& src, CBuffer& dst,
                                   const std::vector<CBufferImageCopy>& regions) = 0;
    virtual void BlitImage(CImage& src, CImage& dst, const std::vector<CImageBlit>& regions,
                           EFilter filter) = 0;
    virtual void ResolveImage(CImage& src, CImage& dst,
                              const std::vector<CImageResolve>& regions) = 0;

    virtual void FinishRecording() = 0;
};

} /* namespace RHI */
