#pragma once
#include "RHIModuleAPI.h"

namespace RHI
{

/* This file is used by the RHI object classes as a compile
 *   time aid to determine which class actually implements it.
 */

template <template <typename> typename TBaseClass> struct TChooseImpl;

#define DEFINE_IMPL(ApiType, ImplType)                                                             \
    template <typename TDerived> class ApiType##Base;                                              \
                                                                                                   \
    class ImplType;                                                                                \
                                                                                                   \
    template <> struct TChooseImpl<ApiType##Base>                                                  \
    {                                                                                              \
        using TDerived = ImplType;                                                                 \
        using TConcreteBase = ApiType##Base<TDerived>;                                             \
    };                                                                                             \
                                                                                                   \
    typedef ApiType##Base<ImplType> ApiType;

#ifdef RHI_IMPL_DIRECT3D11
DEFINE_IMPL(CBuffer, CBufferD3D11)
DEFINE_IMPL(CImage, CImageD3D11)

DEFINE_IMPL(CDevice, CDeviceD3D11)

#elif defined(RHI_IMPL_VULKAN)
// Resources
DEFINE_IMPL(CBuffer, CBufferVk)
DEFINE_IMPL(CImage, CImageVk)

DEFINE_IMPL(CDevice, CDeviceVk)

#else
static_assert(false, "No RHI implementation chosen.");
#endif

} /* namespace RHI */
