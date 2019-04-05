#pragma once
#include "CopyContext.h"
#include "Pipeline.h"

namespace RHI
{

class IRenderContext : public ICopyContext
{
public:
    typedef std::shared_ptr<IRenderContext> Ref;

    virtual ~IRenderContext() = default;

    virtual void BindPipeline(CPipeline* pipeline) = 0;
    virtual void BindBuffer(CBuffer* buffer, size_t offset, size_t range, uint32_t set,
                            uint32_t binding, uint32_t index) = 0;
    virtual void BindBufferView(CBufferView* bufferView, uint32_t set, uint32_t binding,
                                uint32_t index) = 0;
    virtual void BindImageView(CImageView* imageView, uint32_t set, uint32_t binding,
                               uint32_t index) = 0;
    virtual void BindSampler(CSampler* sampler, uint32_t set, uint32_t binding, uint32_t index) = 0;
    virtual void BindIndexBuffer(CBuffer* buffer, size_t offset, EFormat format) = 0;
    virtual void BindVertexBuffer(uint32_t binding, CBuffer* buffer, size_t offset) = 0;
    virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                      uint32_t firstInstance) = 0;
    virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                             int32_t vertexOffset, uint32_t firstInstance) = 0;
};

} /* namespace RHI */
