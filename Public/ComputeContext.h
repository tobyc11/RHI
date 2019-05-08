#pragma once
#include "CopyContext.h"
#include "Pipeline.h"
#include "DescriptorSet.h"

namespace RHI
{

class IComputeContext
{
public:
    typedef std::shared_ptr<IComputeContext> Ref;

    virtual ~IComputeContext() = default;

    virtual void BindComputePipeline(CPipeline& pipeline) = 0;
    virtual void BindComputeDescriptorSet(uint32_t set, CDescriptorSet& descriptorSet) = 0;
    virtual void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
    virtual void DispatchIndirect(CBuffer& buffer, size_t offset) = 0;

    virtual void FinishRecording() = 0;
};

} /* namespace RHI */
