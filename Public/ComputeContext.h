#pragma once
#include "CopyContext.h"
#include "Pipeline.h"

namespace RHI
{

class IComputeContext : public ICopyContext
{
public:
    typedef std::shared_ptr<IComputeContext> Ref;

    virtual ~IComputeContext() = default;

    virtual void BindPipeline(CPipeline& pipeline) = 0;
    virtual void BindBuffer(CBuffer& buffer, size_t offset, size_t range, uint32_t set,
                            uint32_t binding, uint32_t index) = 0;
    virtual void BindBufferView(CBufferView& bufferView, uint32_t set, uint32_t binding,
                                uint32_t index) = 0;
    virtual void BindConstants(const void* pData, size_t size, uint32_t set, uint32_t binding,
                               uint32_t index) = 0;
    virtual void BindImageView(CImageView& imageView, uint32_t set, uint32_t binding,
                               uint32_t index) = 0;
    virtual void BindSampler(CSampler& sampler, uint32_t set, uint32_t binding, uint32_t index) = 0;
    virtual void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
};

} /* namespace RHI */
