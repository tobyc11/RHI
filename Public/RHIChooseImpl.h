#pragma once
#include "RHIModuleAPI.h"

namespace RHI
{

/* This file is used by the RHI object classes as a compile
 *   time aid to determine which class actually implements it.
 */

template <template<typename> typename TBaseClass>
struct TChooseImpl;

#define DEFINE_IMPL(BaseType, ImplType) \
template <typename TDerived> \
class BaseType; \
 \
class ImplType; \
 \
template <> \
struct TChooseImpl<BaseType> \
{ \
    using TDerived = ImplType; \
    using TConcreteBase = BaseType<TDerived>; \
};

#ifdef RHI_IMPL_DIRECT3D11
DEFINE_IMPL(CDeviceBase, CDeviceD3D11)
DEFINE_IMPL(CBufferBase, CBufferD3D11)
DEFINE_IMPL(CImageBase, CImageD3D11)
#else
static_assert(false, "No RHI implementation chosen.");
#endif

} /* namespace RHI */
