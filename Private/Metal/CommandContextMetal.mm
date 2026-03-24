#include "CommandContextMetal.h"
#include "BufferMetal.h"
#include "CommandQueueMetal.h"
#include "DescriptorSetMetal.h"
#include "DeviceMetal.h"
#include "ImageMetal.h"
#include "ImageViewMetal.h"
#include "MtlHelpers.h"
#include "PipelineMetal.h"
#include "RenderPassMetal.h"

namespace RHI
{

// --- CCommandContextMetal ---

CCommandContextMetal::CCommandContextMetal(EContextMode mode, CDeviceMetal& device)
    : Mode(mode)
    , Device(device)
{
}

CCommandContextMetal::Ref CCommandContextMetal::CreateCopyContext(CCommandListMetal& cmdList)
{
    auto ctx = Ref(new CCommandContextMetal(EContextMode::Copy,
                                            cmdList.GetQueue().GetDevice()));
    ctx->BlitEncoder = [(id<MTLCommandBuffer>)cmdList.GetMTLCommandBuffer() blitCommandEncoder];
    return ctx;
}

CCommandContextMetal::Ref CCommandContextMetal::CreateComputeContext(CCommandListMetal& cmdList)
{
    auto ctx = Ref(new CCommandContextMetal(EContextMode::Compute,
                                            cmdList.GetQueue().GetDevice()));
    ctx->ComputeEncoder = [(id<MTLCommandBuffer>)cmdList.GetMTLCommandBuffer() computeCommandEncoder];
    return ctx;
}

CCommandContextMetal::Ref CCommandContextMetal::CreateRenderContext(
    id encoder, CDeviceMetal& device)
{
    auto ctx = Ref(new CCommandContextMetal(EContextMode::Render, device));
    ctx->RenderEncoder = encoder;
    return ctx;
}

CCommandContextMetal::~CCommandContextMetal() { }

// --- ICopyContext ---

void CCommandContextMetal::ClearImage(CImage& image, const CClearValue& clearValue,
                                      const CImageSubresourceRange& range)
{
    // Metal doesn't have a direct blit clear; would need a render pass or compute shader
    // For simplicity, this is a no-op placeholder
}

void CCommandContextMetal::CopyBuffer(CBuffer& src, CBuffer& dst,
                                      const std::vector<CBufferCopy>& regions)
{
    auto& srcBuf = static_cast<CBufferMetal&>(src);
    auto& dstBuf = static_cast<CBufferMetal&>(dst);

    for (const auto& region : regions)
    {
        [BlitEncoder copyFromBuffer:srcBuf.GetMTLBuffer()
                       sourceOffset:region.SrcOffset
                           toBuffer:dstBuf.GetMTLBuffer()
                  destinationOffset:region.DstOffset
                               size:region.Size];
    }
}

void CCommandContextMetal::CopyImage(CImage& src, CImage& dst,
                                     const std::vector<CImageCopy>& regions)
{
    auto& srcImg = static_cast<CImageMetal&>(src);
    auto& dstImg = static_cast<CImageMetal&>(dst);

    for (const auto& region : regions)
    {
        [BlitEncoder copyFromTexture:srcImg.GetMTLTexture()
                         sourceSlice:region.SrcSubresource.BaseArrayLayer
                         sourceLevel:region.SrcSubresource.MipLevel
                        sourceOrigin:MTLOriginMake(region.SrcOffset.X, region.SrcOffset.Y,
                                                   region.SrcOffset.Z)
                          sourceSize:MTLSizeMake(region.Extent.Width, region.Extent.Height,
                                                 region.Extent.Depth)
                           toTexture:dstImg.GetMTLTexture()
                    destinationSlice:region.DstSubresource.BaseArrayLayer
                    destinationLevel:region.DstSubresource.MipLevel
                   destinationOrigin:MTLOriginMake(region.DstOffset.X, region.DstOffset.Y,
                                                   region.DstOffset.Z)];
    }
}

void CCommandContextMetal::CopyBufferToImage(CBuffer& src, CImage& dst,
                                             const std::vector<CBufferImageCopy>& regions)
{
    auto& srcBuf = static_cast<CBufferMetal&>(src);
    auto& dstImg = static_cast<CImageMetal&>(dst);

    for (const auto& region : regions)
    {
        NSUInteger bytesPerRow = region.BufferRowLength > 0
            ? region.BufferRowLength
            : region.ImageExtent.Width * 4;
        NSUInteger bytesPerImage = region.BufferImageHeight > 0
            ? bytesPerRow * region.BufferImageHeight
            : bytesPerRow * region.ImageExtent.Height;

        [BlitEncoder copyFromBuffer:srcBuf.GetMTLBuffer()
                       sourceOffset:region.BufferOffset
                  sourceBytesPerRow:bytesPerRow
                sourceBytesPerImage:bytesPerImage
                         sourceSize:MTLSizeMake(region.ImageExtent.Width,
                                                region.ImageExtent.Height,
                                                region.ImageExtent.Depth)
                          toTexture:dstImg.GetMTLTexture()
                   destinationSlice:region.ImageSubresource.BaseArrayLayer
                   destinationLevel:region.ImageSubresource.MipLevel
                  destinationOrigin:MTLOriginMake(region.ImageOffset.X, region.ImageOffset.Y,
                                                  region.ImageOffset.Z)];
    }
}

void CCommandContextMetal::CopyImageToBuffer(CImage& src, CBuffer& dst,
                                             const std::vector<CBufferImageCopy>& regions)
{
    auto& srcImg = static_cast<CImageMetal&>(src);
    auto& dstBuf = static_cast<CBufferMetal&>(dst);

    for (const auto& region : regions)
    {
        NSUInteger bytesPerRow = region.BufferRowLength > 0
            ? region.BufferRowLength
            : region.ImageExtent.Width * 4;
        NSUInteger bytesPerImage = region.BufferImageHeight > 0
            ? bytesPerRow * region.BufferImageHeight
            : bytesPerRow * region.ImageExtent.Height;

        [BlitEncoder copyFromTexture:srcImg.GetMTLTexture()
                         sourceSlice:region.ImageSubresource.BaseArrayLayer
                         sourceLevel:region.ImageSubresource.MipLevel
                        sourceOrigin:MTLOriginMake(region.ImageOffset.X, region.ImageOffset.Y,
                                                   region.ImageOffset.Z)
                          sourceSize:MTLSizeMake(region.ImageExtent.Width,
                                                 region.ImageExtent.Height,
                                                 region.ImageExtent.Depth)
                            toBuffer:dstBuf.GetMTLBuffer()
                   destinationOffset:region.BufferOffset
              destinationBytesPerRow:bytesPerRow
            destinationBytesPerImage:bytesPerImage];
    }
}

void CCommandContextMetal::BlitImage(CImage& src, CImage& dst,
                                     const std::vector<CImageBlit>& regions, EFilter filter)
{
    // Metal blit encoder doesn't support scaled blit directly; would need a render pass
    // For now, do a basic copy for each region
    auto& srcImg = static_cast<CImageMetal&>(src);
    auto& dstImg = static_cast<CImageMetal&>(dst);

    for (const auto& region : regions)
    {
        MTLOrigin srcOrigin = MTLOriginMake(region.SrcOffsets[0].X, region.SrcOffsets[0].Y,
                                            region.SrcOffsets[0].Z);
        MTLSize srcSize = MTLSizeMake(
            region.SrcOffsets[1].X - region.SrcOffsets[0].X,
            region.SrcOffsets[1].Y - region.SrcOffsets[0].Y,
            std::max(1, region.SrcOffsets[1].Z - region.SrcOffsets[0].Z));
        MTLOrigin dstOrigin = MTLOriginMake(region.DstOffsets[0].X, region.DstOffsets[0].Y,
                                            region.DstOffsets[0].Z);

        [BlitEncoder copyFromTexture:srcImg.GetMTLTexture()
                         sourceSlice:region.SrcSubresource.BaseArrayLayer
                         sourceLevel:region.SrcSubresource.MipLevel
                        sourceOrigin:srcOrigin
                          sourceSize:srcSize
                           toTexture:dstImg.GetMTLTexture()
                    destinationSlice:region.DstSubresource.BaseArrayLayer
                    destinationLevel:region.DstSubresource.MipLevel
                   destinationOrigin:dstOrigin];
    }
}

void CCommandContextMetal::ResolveImage(CImage& src, CImage& dst,
                                        const std::vector<CImageResolve>& regions)
{
    // Resolve is typically done via render pass store action in Metal
    // Placeholder implementation
}

// --- IComputeContext ---

void CCommandContextMetal::BindComputePipeline(CPipeline& pipeline)
{
    BoundComputePipeline = static_cast<CPipelineMetal*>(&pipeline);
    [ComputeEncoder setComputePipelineState:BoundComputePipeline->GetComputePipelineState()];
}

void CCommandContextMetal::BindComputeDescriptorSet(uint32_t set,
                                                    CDescriptorSet& descriptorSet)
{
    if (!BoundComputePipeline)
        return;
    auto& ds = static_cast<CDescriptorSetMetal&>(descriptorSet);
    ds.ApplyToComputeEncoder(ComputeEncoder, BoundComputePipeline->GetCSRemap(), set);
}

void CCommandContextMetal::Dispatch(uint32_t groupCountX, uint32_t groupCountY,
                                    uint32_t groupCountZ)
{
    if (!BoundComputePipeline)
        return;

    id<MTLComputePipelineState> cps = (id<MTLComputePipelineState>)BoundComputePipeline->GetComputePipelineState();
    NSUInteger w = cps.threadExecutionWidth;
    NSUInteger h = cps.maxTotalThreadsPerThreadgroup / w;

    MTLSize threadsPerGroup = MTLSizeMake(w, h, 1);
    MTLSize threadgroups = MTLSizeMake(groupCountX, groupCountY, groupCountZ);

    [ComputeEncoder dispatchThreadgroups:threadgroups threadsPerThreadgroup:threadsPerGroup];
}

void CCommandContextMetal::DispatchIndirect(CBuffer& buffer, size_t offset)
{
    if (!BoundComputePipeline)
        return;

    auto& buf = static_cast<CBufferMetal&>(buffer);
    id<MTLComputePipelineState> cps = (id<MTLComputePipelineState>)BoundComputePipeline->GetComputePipelineState();
    NSUInteger w = cps.threadExecutionWidth;
    NSUInteger h = cps.maxTotalThreadsPerThreadgroup / w;
    MTLSize threadsPerGroup = MTLSizeMake(w, h, 1);

    [ComputeEncoder dispatchThreadgroupsWithIndirectBuffer:buf.GetMTLBuffer()
                                      indirectBufferOffset:offset
                                     threadsPerThreadgroup:threadsPerGroup];
}

// --- IRenderContext ---

void CCommandContextMetal::BindRenderPipeline(CPipeline& pipeline)
{
    BoundRenderPipeline = static_cast<CPipelineMetal*>(&pipeline);
    [RenderEncoder setRenderPipelineState:BoundRenderPipeline->GetRenderPipelineState()];
    [RenderEncoder setDepthStencilState:BoundRenderPipeline->GetDepthStencilState()];
    [RenderEncoder setCullMode:BoundRenderPipeline->GetCullMode()];
    [RenderEncoder setFrontFacingWinding:BoundRenderPipeline->GetWinding()];
    [RenderEncoder setTriangleFillMode:BoundRenderPipeline->GetFillMode()];

    if (BoundRenderPipeline->GetDepthBiasEnable())
    {
        [RenderEncoder setDepthBias:BoundRenderPipeline->GetDepthBiasConstant()
                         slopeScale:BoundRenderPipeline->GetDepthBiasSlope()
                              clamp:BoundRenderPipeline->GetDepthBiasClamp()];
    }
}

void CCommandContextMetal::SetViewport(const CViewportDesc& viewportDesc)
{
    MTLViewport vp;
    vp.originX = viewportDesc.X;
    vp.originY = viewportDesc.Y;
    vp.width = viewportDesc.Width;
    vp.height = viewportDesc.Height;
    vp.znear = viewportDesc.MinDepth;
    vp.zfar = viewportDesc.MaxDepth;
    [RenderEncoder setViewport:vp];
}

void CCommandContextMetal::SetScissor(const CRect2D& scissor)
{
    MTLScissorRect rect;
    rect.x = std::max(0, scissor.Offset.X);
    rect.y = std::max(0, scissor.Offset.Y);
    rect.width = scissor.Extent.Width;
    rect.height = scissor.Extent.Height;
    [RenderEncoder setScissorRect:rect];
}

void CCommandContextMetal::SetBlendConstants(const std::array<float, 4>& blendConstants)
{
    [RenderEncoder setBlendColorRed:blendConstants[0]
                              green:blendConstants[1]
                               blue:blendConstants[2]
                              alpha:blendConstants[3]];
}

void CCommandContextMetal::SetStencilReference(uint32_t reference)
{
    [RenderEncoder setStencilReferenceValue:reference];
}

void CCommandContextMetal::BindRenderDescriptorSet(uint32_t set,
                                                   CDescriptorSet& descriptorSet)
{
    if (!BoundRenderPipeline)
        return;

    auto& ds = static_cast<CDescriptorSetMetal&>(descriptorSet);
    // Apply VS remap
    ds.ApplyToRenderEncoder(RenderEncoder, BoundRenderPipeline->GetVSRemap(), set);
    // PS remap is also applied (done inside ApplyToRenderEncoder which sets both)
}

void CCommandContextMetal::BindIndexBuffer(CBuffer& buffer, size_t offset, EFormat format)
{
    auto& buf = static_cast<CBufferMetal&>(buffer);
    BoundIndexBuffer = buf.GetMTLBuffer();
    BoundIndexOffset = offset;
    BoundIndexType = MtlIndexType(format);
}

void CCommandContextMetal::BindVertexBuffer(uint32_t binding, CBuffer& buffer, size_t offset)
{
    auto& buf = static_cast<CBufferMetal&>(buffer);
    [RenderEncoder setVertexBuffer:buf.GetMTLBuffer() offset:offset atIndex:binding];
}

void CCommandContextMetal::Draw(uint32_t vertexCount, uint32_t instanceCount,
                                uint32_t firstVertex, uint32_t firstInstance)
{
    if (!BoundRenderPipeline)
        return;
    [RenderEncoder drawPrimitives:BoundRenderPipeline->GetPrimitiveType()
                      vertexStart:firstVertex
                      vertexCount:vertexCount
                    instanceCount:instanceCount
                     baseInstance:firstInstance];
}

void CCommandContextMetal::DrawIndexed(uint32_t indexCount, uint32_t instanceCount,
                                       uint32_t firstIndex, int32_t vertexOffset,
                                       uint32_t firstInstance)
{
    if (!BoundRenderPipeline || !BoundIndexBuffer)
        return;

    NSUInteger indexSize = (BoundIndexType == MTLIndexTypeUInt16) ? 2 : 4;
    NSUInteger indexBufferOffset = BoundIndexOffset + firstIndex * indexSize;

    [RenderEncoder drawIndexedPrimitives:BoundRenderPipeline->GetPrimitiveType()
                              indexCount:indexCount
                               indexType:BoundIndexType
                             indexBuffer:BoundIndexBuffer
                       indexBufferOffset:indexBufferOffset
                           instanceCount:instanceCount
                              baseVertex:vertexOffset
                            baseInstance:firstInstance];
}

void CCommandContextMetal::DrawIndirect(CBuffer& buffer, size_t offset, uint32_t drawCount,
                                        uint32_t stride)
{
    if (!BoundRenderPipeline)
        return;

    auto& buf = static_cast<CBufferMetal&>(buffer);
    for (uint32_t i = 0; i < drawCount; ++i)
    {
        [RenderEncoder drawPrimitives:BoundRenderPipeline->GetPrimitiveType()
                       indirectBuffer:buf.GetMTLBuffer()
                 indirectBufferOffset:offset + i * stride];
    }
}

void CCommandContextMetal::DrawIndexedIndirect(CBuffer& buffer, size_t offset,
                                               uint32_t drawCount, uint32_t stride)
{
    if (!BoundRenderPipeline || !BoundIndexBuffer)
        return;

    auto& buf = static_cast<CBufferMetal&>(buffer);
    for (uint32_t i = 0; i < drawCount; ++i)
    {
        [RenderEncoder drawIndexedPrimitives:BoundRenderPipeline->GetPrimitiveType()
                                   indexType:BoundIndexType
                                 indexBuffer:BoundIndexBuffer
                           indexBufferOffset:BoundIndexOffset
                              indirectBuffer:buf.GetMTLBuffer()
                        indirectBufferOffset:offset + i * stride];
    }
}

void CCommandContextMetal::FinishRecording()
{
    if (BlitEncoder)
        [BlitEncoder endEncoding];
    if (ComputeEncoder)
        [ComputeEncoder endEncoding];
    if (RenderEncoder)
        [RenderEncoder endEncoding];
}

// --- CRenderPassContextMetal ---

CRenderPassContextMetal::CRenderPassContextMetal(CCommandListMetal& cmdList,
                                                 CRenderPass::Ref renderPass,
                                                 const std::vector<CClearValue>& clearValues)
    : CmdList(cmdList)
    , RenderPass(renderPass)
    , ClearValues(clearValues)
{
    auto* mtlRP = static_cast<CRenderPassMetal*>(renderPass.get());

    if (mtlRP->GetSubpassCount() == 1)
    {
        MTLRenderPassDescriptor* rpd = mtlRP->CreateMTLRenderPassDescriptor(0, clearValues);
        ParallelEncoder =
            [cmdList.GetMTLCommandBuffer() parallelRenderCommandEncoderWithDescriptor:rpd];
    }
}

CRenderPassContextMetal::~CRenderPassContextMetal() { }

IRenderContext::Ref CRenderPassContextMetal::CreateRenderContext(uint32_t subpass)
{
    auto* mtlRP = static_cast<CRenderPassMetal*>(RenderPass.get());

    if (subpass != CurrentSubpass && subpass > 0)
    {
        // Metal doesn't have subpass transitions within a single render pass.
        // For multi-subpass, we'd need to end the current pass and start a new one.
        // For the common single-subpass case, we just use the existing parallel encoder.
        if (ParallelEncoder)
        {
            [ParallelEncoder endEncoding];
            ParallelEncoder = nil;
        }

        MTLRenderPassDescriptor* rpd =
            mtlRP->CreateMTLRenderPassDescriptor(subpass, ClearValues);
        ParallelEncoder =
            [CmdList.GetMTLCommandBuffer() parallelRenderCommandEncoderWithDescriptor:rpd];
        CurrentSubpass = subpass;
    }

    if (!ParallelEncoder)
    {
        MTLRenderPassDescriptor* rpd =
            mtlRP->CreateMTLRenderPassDescriptor(subpass, ClearValues);
        ParallelEncoder =
            [CmdList.GetMTLCommandBuffer() parallelRenderCommandEncoderWithDescriptor:rpd];
        CurrentSubpass = subpass;
    }

    id<MTLRenderCommandEncoder> encoder = [ParallelEncoder renderCommandEncoder];
    return CCommandContextMetal::CreateRenderContext(encoder, CmdList.GetQueue().GetDevice());
}

void CRenderPassContextMetal::FinishRecording()
{
    if (ParallelEncoder)
    {
        [ParallelEncoder endEncoding];
        ParallelEncoder = nil;
    }
}

} /* namespace RHI */
