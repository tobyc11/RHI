#include "BufferMetal.h"
#include "DeviceMetal.h"
#include "MtlHelpers.h"
#include "RHIException.h"

namespace RHI
{

CBufferMetal::CBufferMetal(CDeviceMetal& parent, size_t size, EBufferUsageFlags usage,
                           const void* initialData)
    : CBuffer(size, usage)
    , Parent(parent)
{
    id<MTLDevice> device = (id<MTLDevice>)parent.GetMTLDevice();
    MTLResourceOptions options;
    auto usageVal = static_cast<uint32_t>(usage);

    bool isUpload = usageVal & static_cast<uint32_t>(EBufferUsageFlags::Upload);
    bool isReadback = usageVal & static_cast<uint32_t>(EBufferUsageFlags::Readback);
    bool isDynamic = usageVal & static_cast<uint32_t>(EBufferUsageFlags::Dynamic);

    if (isUpload || isDynamic)
        options = MTLResourceStorageModeShared | MTLResourceCPUCacheModeWriteCombined;
    else if (isReadback)
        options = MTLResourceStorageModeShared;
    else
        options = MTLResourceStorageModePrivate;

    if (initialData && (options & MTLResourceStorageModePrivate))
    {
        id<MTLBuffer> staging = [device newBufferWithBytes:initialData
                                                    length:size
                                                   options:MTLResourceStorageModeShared];
        Buffer = [device newBufferWithLength:size options:options];

        id<MTLCommandQueue> queue = (id<MTLCommandQueue>)parent.GetDefaultQueue();
        id<MTLCommandBuffer> cmdBuf = [queue commandBuffer];
        id<MTLBlitCommandEncoder> blit = [cmdBuf blitCommandEncoder];
        [blit copyFromBuffer:staging sourceOffset:0
                    toBuffer:(id<MTLBuffer>)Buffer destinationOffset:0 size:size];
        [blit endEncoding];
        [cmdBuf commit];
        [cmdBuf waitUntilCompleted];
    }
    else if (initialData)
    {
        Buffer = [device newBufferWithBytes:initialData length:size options:options];
    }
    else
    {
        Buffer = [device newBufferWithLength:size options:options];
    }

    if (!Buffer)
        throw CRHIRuntimeError("Failed to create Metal buffer");
}

CBufferMetal::~CBufferMetal()
{
    id buf = Buffer;
    Parent.AddPostFrameCleanup([buf](CDeviceMetal&) { (void)buf; });
}

void* CBufferMetal::Map(size_t offset, size_t size)
{
    MappedOffset = offset;
    MappedSize = size;
    return static_cast<uint8_t*>([(id<MTLBuffer>)Buffer contents]) + offset;
}

void CBufferMetal::Unmap()
{
    id<MTLBuffer> buf = (id<MTLBuffer>)Buffer;
    if (buf.storageMode == MTLStorageModeManaged)
    {
        [buf didModifyRange:NSMakeRange(MappedOffset, MappedSize)];
    }
    MappedOffset = 0;
    MappedSize = 0;
}

} /* namespace RHI */
