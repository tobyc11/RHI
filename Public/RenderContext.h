#pragma once
#include "ComputeContext.h"

namespace RHI
{

struct CClearDepthStencilValue
{
    float Depth;
    uint32_t Stencil;
};

struct CClearValue
{
    union
    {
        union
        {
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
        : DepthStencilValue{d, s}
    {
    }
};

class IRenderContext
{
public:
    typedef std::shared_ptr<IRenderContext> Ref;

    virtual ~IRenderContext() = default;

    virtual void BindRenderPipeline(CPipeline& pipeline) = 0;
    virtual void SetViewport(const CViewportDesc& viewportDesc) = 0;
    virtual void SetScissor(const CRect2D& scissor) = 0;
    virtual void SetBlendConstants(const std::array<float, 4>& blendConstants) = 0;
    virtual void SetStencilReference(uint32_t reference) = 0;
    virtual void BindRenderDescriptorSet(uint32_t set, CDescriptorSet& descriptorSet) = 0;
    virtual void BindIndexBuffer(CBuffer& buffer, size_t offset, EFormat format) = 0;
    virtual void BindVertexBuffer(uint32_t binding, CBuffer& buffer, size_t offset) = 0;
    virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                      uint32_t firstInstance) = 0;
    virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount,
                             uint32_t firstIndex, int32_t vertexOffset,
                             uint32_t firstInstance) = 0;
    virtual void DrawIndirect(CBuffer& buffer, size_t offset, uint32_t drawCount, uint32_t stride) = 0;
    virtual void DrawIndexedIndirect(CBuffer& buffer, size_t offset, uint32_t drawCount, uint32_t stride) = 0;

    virtual void FinishRecording() = 0;
};

// A meta context from which you can create multiple render contexts for a render pass
class IRenderPassContext
{
public:
    typedef std::shared_ptr<IRenderPassContext> Ref;
    virtual ~IRenderPassContext() = default;
    virtual IRenderContext::Ref CreateRenderContext(uint32_t subpass) = 0;
    virtual void FinishRecording() = 0;
};

} /* namespace RHI */
