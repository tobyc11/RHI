#pragma once
#include "PipelineCache.h"
#ifdef RHI_IMPL_DIRECT3D11
#include "Direct3D11/PipelineCacheD3D11.h"
#endif

namespace RHI
{

template<typename TDerived>
CPipelineStates CPipelineCacheBase<TDerived>::CreatePipelineStates(const CDrawTemplate& dt)
{
    return static_cast<TDerived*>(this)->CreatePipelineStates(dt);
}

template<typename TDerived>
void CPipelineCacheBase<TDerived>::DestroyPipelineStates(CPipelineStates pso)
{
    return static_cast<TDerived*>(this)->DestroyPipelineStates(pso);
}

//Explicitly instanciate the wrapper for the chosen implementation
template class RHI_API CPipelineCacheBase<TChooseImpl<CPipelineCacheBase>::TDerived>;

} /* namespace RHI */
