#include "DeviceMetal.h"
#include "AbstractionBreaker.h"
#include "BufferMetal.h"
#include "CommandQueueMetal.h"
#include "DescriptorSetMetal.h"
#include "ImageMetal.h"
#include "ImageViewMetal.h"
#include "MtlHelpers.h"
#include "PipelineMetal.h"
#include "PresentationSurfaceDesc.h"
#include "RenderPassMetal.h"
#include "SamplerMetal.h"
#include "ShaderModuleMetal.h"
#include "SwapChainMetal.h"

namespace RHI
{

void InitRHIInstance() { }

void ShutdownRHIInstance() { }

CDeviceMetal::CDeviceMetal(EDeviceCreateHints hints)
{
    NSArray<id<MTLDevice>>* devices = MTLCopyAllDevices();
    if (devices.count == 0)
        throw CRHIRuntimeError("No Metal devices available");

    Device = devices[0];

    if (hints == EDeviceCreateHints::Integrated || hints == EDeviceCreateHints::Discrete)
    {
        bool wantLowPower = (hints == EDeviceCreateHints::Integrated);
        for (id<MTLDevice> d in devices)
        {
            if (d.isLowPower == wantLowPower)
            {
                Device = d;
                break;
            }
        }
    }

    DefaultQueue = [(id<MTLDevice>)Device newCommandQueue];
    if (!DefaultQueue)
        throw CRHIRuntimeError("Failed to create Metal command queue");
}

CDeviceMetal::~CDeviceMetal() { }

CBuffer::Ref CDeviceMetal::CreateBuffer(size_t size, EBufferUsageFlags usage,
                                        const void* initialData)
{
    return std::make_shared<CBufferMetal>(*this, size, usage, initialData);
}

CImage::Ref CDeviceMetal::InternalCreateImage(unsigned long type, EFormat format,
                                              EImageUsageFlags usage, uint32_t width,
                                              uint32_t height, uint32_t depth, uint32_t mipLevels,
                                              uint32_t arrayLayers, uint32_t sampleCount,
                                              const void* initialData)
{
    return std::make_shared<CMemoryImageMetal>(*this, type, format, usage, width, height, depth,
                                               mipLevels, arrayLayers, sampleCount, initialData);
}

CImage::Ref CDeviceMetal::CreateImage1D(EFormat format, EImageUsageFlags usage, uint32_t width,
                                        uint32_t mipLevels, uint32_t arrayLayers,
                                        uint32_t sampleCount, const void* initialData)
{
    MTLTextureType type =
        arrayLayers > 1 ? MTLTextureType1DArray : MTLTextureType1D;
    return InternalCreateImage(type, format, usage, width, 1, 1, mipLevels, arrayLayers,
                               sampleCount, initialData);
}

CImage::Ref CDeviceMetal::CreateImage2D(EFormat format, EImageUsageFlags usage, uint32_t width,
                                        uint32_t height, uint32_t mipLevels, uint32_t arrayLayers,
                                        uint32_t sampleCount, const void* initialData)
{
    bool isCube = static_cast<uint32_t>(usage) & static_cast<uint32_t>(EImageUsageFlags::CubeMap);
    MTLTextureType type;
    if (isCube)
        type = arrayLayers > 6 ? MTLTextureTypeCubeArray : MTLTextureTypeCube;
    else if (sampleCount > 1)
        type = MTLTextureType2DMultisample;
    else
        type = arrayLayers > 1 ? MTLTextureType2DArray : MTLTextureType2D;
    return InternalCreateImage(type, format, usage, width, height, 1, mipLevels, arrayLayers,
                               sampleCount, initialData);
}

CImage::Ref CDeviceMetal::CreateImage3D(EFormat format, EImageUsageFlags usage, uint32_t width,
                                        uint32_t height, uint32_t depth, uint32_t mipLevels,
                                        uint32_t arrayLayers, uint32_t sampleCount,
                                        const void* initialData)
{
    return InternalCreateImage(MTLTextureType3D, format, usage, width, height, depth, mipLevels,
                               arrayLayers, sampleCount, initialData);
}

CImageView::Ref CDeviceMetal::CreateImageView(const CImageViewDesc& desc, CImage::Ref image)
{
    return std::make_shared<CImageViewMetal>(desc, image);
}

CShaderModule::Ref CDeviceMetal::CreateShaderModule(size_t size, const void* pCode)
{
    return std::make_shared<CShaderModuleMetal>(*this, size, pCode);
}

CDescriptorSetLayout::Ref
CDeviceMetal::CreateDescriptorSetLayout(const std::vector<CDescriptorSetLayoutBinding>& bindings)
{
    return std::make_shared<CDescriptorSetLayoutMetal>(*this, bindings);
}

CPipelineLayout::Ref
CDeviceMetal::CreatePipelineLayout(const std::vector<CDescriptorSetLayout::Ref>& setLayouts)
{
    return std::make_shared<CPipelineLayoutMetal>(setLayouts);
}

CRenderPass::Ref CDeviceMetal::CreateRenderPass(const CRenderPassDesc& desc)
{
    return std::make_shared<CRenderPassMetal>(*this, desc);
}

CPipeline::Ref CDeviceMetal::CreatePipeline(const CPipelineDesc& desc)
{
    return std::make_shared<CPipelineMetal>(*this, desc);
}

CPipeline::Ref CDeviceMetal::CreateComputePipeline(const CComputePipelineDesc& desc)
{
    return std::make_shared<CPipelineMetal>(*this, desc);
}

CSampler::Ref CDeviceMetal::CreateSampler(const CSamplerDesc& desc)
{
    return std::make_shared<CSamplerMetal>(*this, desc);
}

CCommandQueue::Ref CDeviceMetal::CreateCommandQueue()
{
    return std::make_shared<CCommandQueueMetal>(*this);
}

CSwapChain::Ref CDeviceMetal::CreateSwapChain(const CPresentationSurfaceDesc& info, EFormat format)
{
    return std::make_shared<CSwapChainMetal>(*this, info, format);
}

void CDeviceMetal::WaitIdle()
{
    id<MTLCommandBuffer> cmdBuf = [(id<MTLCommandQueue>)DefaultQueue commandBuffer];
    [cmdBuf commit];
    [cmdBuf waitUntilCompleted];
}

void CDeviceMetal::AddPostFrameCleanup(std::function<void(CDeviceMetal&)> callback)
{
    std::lock_guard<std::mutex> lock(DeviceMutex);
    PostFrameCleanup.push_back(std::move(callback));
}

CNativeDevice GetNativeDevice(CDevice::Ref device)
{
    auto* d = static_cast<CDeviceMetal*>(device.get());
    CNativeDevice native;
    native.Device = (__bridge void*)((id<MTLDevice>)d->GetMTLDevice());
    native.CommandQueue = (__bridge void*)((id<MTLCommandQueue>)d->GetDefaultQueue());
    return native;
}

} /* namespace RHI */
