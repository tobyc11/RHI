#pragma once
#include "RHIChooseImpl.h"
#include "DrawTemplate.h"

namespace RHI
{

using CPipelineStates = void*;

template <typename TDerived>
class CPipelineCacheBase : public tc::CVirtualLightRefBase
{
protected:
    CPipelineCacheBase() = default;

public:
    CPipelineStates CreatePipelineStates(const CDrawTemplate& dt);
    void DestroyPipelineStates(CPipelineStates pso);
};

} /* namespace RHI */
