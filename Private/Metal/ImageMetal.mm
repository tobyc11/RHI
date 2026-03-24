#include "ImageMetal.h"
#include "DeviceMetal.h"
#include "MtlHelpers.h"
#include "RHIException.h"

namespace RHI
{

CMemoryImageMetal::CMemoryImageMetal(CDeviceMetal& parent, unsigned long type, EFormat format,
                                     EImageUsageFlags usage, uint32_t width, uint32_t height,
                                     uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers,
                                     uint32_t sampleCount, const void* initialData)
    : Parent(parent)
{
    Format = format;
    Usage = usage;
    Width = width;
    Height = height;
    Depth = depth;
    MipLevels = mipLevels;
    ArrayLayers = arrayLayers;
    SampleCount = sampleCount;

    id<MTLDevice> device = (id<MTLDevice>)parent.GetMTLDevice();
    MTLTextureType texType = (MTLTextureType)type;

    MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
    desc.textureType = texType;
    desc.pixelFormat = MtlCast(format);
    desc.width = width;
    desc.height = height;
    desc.depth = depth;
    desc.mipmapLevelCount = mipLevels;
    desc.sampleCount = sampleCount;

    if (texType == MTLTextureType1DArray || texType == MTLTextureType2DArray ||
        texType == MTLTextureTypeCubeArray)
        desc.arrayLength = arrayLayers;
    else if (texType == MTLTextureTypeCube)
        desc.arrayLength = 1;

    auto usageVal = static_cast<uint32_t>(usage);
    MTLTextureUsage mtlUsage = MTLTextureUsageUnknown;
    if (usageVal & static_cast<uint32_t>(EImageUsageFlags::Sampled))
        mtlUsage |= MTLTextureUsageShaderRead;
    if (usageVal & static_cast<uint32_t>(EImageUsageFlags::Storage))
        mtlUsage |= MTLTextureUsageShaderWrite;
    if (usageVal & static_cast<uint32_t>(EImageUsageFlags::RenderTarget))
        mtlUsage |= MTLTextureUsageRenderTarget;
    if (usageVal & static_cast<uint32_t>(EImageUsageFlags::DepthStencil))
        mtlUsage |= MTLTextureUsageRenderTarget;
    if (mtlUsage == MTLTextureUsageUnknown)
        mtlUsage = MTLTextureUsageShaderRead;
    desc.usage = mtlUsage;

    bool isStaging = usageVal & static_cast<uint32_t>(EImageUsageFlags::Staging);
    desc.storageMode = isStaging ? MTLStorageModeShared : MTLStorageModePrivate;

    Texture = [device newTextureWithDescriptor:desc];
    if (!Texture)
        throw CRHIRuntimeError("Failed to create Metal texture");

    if (initialData && texType == MTLTextureType2D && mipLevels == 1)
    {
        NSUInteger bytesPerPixel = 4;
        MTLPixelFormat pf = MtlCast(format);
        if (pf == MTLPixelFormatR8Unorm || pf == MTLPixelFormatR8Uint || pf == MTLPixelFormatR8Sint)
            bytesPerPixel = 1;
        else if (pf == MTLPixelFormatRG8Unorm || pf == MTLPixelFormatR16Float)
            bytesPerPixel = 2;
        else if (pf == MTLPixelFormatRGBA16Float)
            bytesPerPixel = 8;
        else if (pf == MTLPixelFormatRGBA32Float)
            bytesPerPixel = 16;

        NSUInteger bytesPerRow = width * bytesPerPixel;
        NSUInteger totalBytes = bytesPerRow * height;

        if (isStaging || desc.storageMode == MTLStorageModeShared)
        {
            [(id<MTLTexture>)Texture replaceRegion:MTLRegionMake2D(0, 0, width, height)
                                       mipmapLevel:0
                                         withBytes:initialData
                                       bytesPerRow:bytesPerRow];
        }
        else
        {
            id<MTLBuffer> staging =
                [device newBufferWithBytes:initialData
                                    length:totalBytes
                                   options:MTLResourceStorageModeShared];
            id<MTLCommandQueue> queue = (id<MTLCommandQueue>)parent.GetDefaultQueue();
            id<MTLCommandBuffer> cmdBuf = [queue commandBuffer];
            id<MTLBlitCommandEncoder> blit = [cmdBuf blitCommandEncoder];
            [blit copyFromBuffer:staging
                    sourceOffset:0
               sourceBytesPerRow:bytesPerRow
             sourceBytesPerImage:totalBytes
                      sourceSize:MTLSizeMake(width, height, 1)
                       toTexture:(id<MTLTexture>)Texture
                destinationSlice:0
                destinationLevel:0
               destinationOrigin:MTLOriginMake(0, 0, 0)];
            [blit endEncoding];
            [cmdBuf commit];
            [cmdBuf waitUntilCompleted];
        }
    }
}

CMemoryImageMetal::~CMemoryImageMetal()
{
    id tex = Texture;
    Parent.AddPostFrameCleanup([tex](CDeviceMetal&) { (void)tex; });
}

CSwapChainImageMetal::CSwapChainImageMetal(EFormat format, uint32_t width, uint32_t height)
{
    Format = format;
    Usage = EImageUsageFlags::RenderTarget;
    Width = width;
    Height = height;
    Texture = nil;
}

void CSwapChainImageMetal::SetTexture(id tex) { Texture = tex; }

void CSwapChainImageMetal::SetSize(uint32_t w, uint32_t h)
{
    Width = w;
    Height = h;
}

} /* namespace RHI */
