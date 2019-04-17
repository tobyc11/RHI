#pragma once
#include "CopyContext.h"
#include "Pipeline.h"

namespace RHI
{

struct CClearValue
{
    union {
        float ColorFloat32[4];
        int32_t ColorInt32[4];
        uint32_t ColorUInt32[4];
    };
    float Depth;
    uint32_t Stencil;

    CClearValue(float r, float g, float b, float a)
    {
        ColorFloat32[0] = r;
        ColorFloat32[1] = g;
        ColorFloat32[2] = b;
        ColorFloat32[3] = a;
    }

    CClearValue(float d, uint32_t s)
        : Depth(d)
        , Stencil(s)
    {
    }
};

class IRenderContext : public ICopyContext
{
public:
    typedef std::shared_ptr<IRenderContext> Ref;

    virtual ~IRenderContext() = default;

    virtual void BeginRenderPass(CRenderPass& renderPass,
                                 const std::vector<CClearValue>& clearValues) = 0;
    virtual void NextSubpass() = 0;
    virtual void EndRenderPass() = 0;

    virtual void BindPipeline(CPipeline& pipeline) = 0;
    // Set Viewport Scissor BlendFactor StencilRef
    virtual void BindBuffer(CBuffer& buffer, size_t offset, size_t range, uint32_t set,
                            uint32_t binding, uint32_t index) = 0;
    virtual void BindBufferView(CBufferView& bufferView, uint32_t set, uint32_t binding,
                                uint32_t index) = 0;
    virtual void BindConstants(const void* pData, size_t size, uint32_t set, uint32_t binding,
                               uint32_t index) = 0;
    virtual void BindImageView(CImageView& imageView, uint32_t set, uint32_t binding,
                               uint32_t index) = 0;
    virtual void BindSampler(CSampler& sampler, uint32_t set, uint32_t binding, uint32_t index) = 0;
    virtual void BindIndexBuffer(CBuffer& buffer, size_t offset, EFormat format) = 0;
    virtual void BindVertexBuffer(uint32_t binding, CBuffer& buffer, size_t offset) = 0;
    virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                      uint32_t firstInstance) = 0;
    virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                             int32_t vertexOffset, uint32_t firstInstance) = 0;
};

} /* namespace RHI */
