#pragma once
#include "RHIChooseImpl.h"
#include "Format.h"
#include <EnumClass.h>
#include <RefBase.h>

namespace RHI
{

enum class EImageType
{
    Image1D,
    Image2D,
    Image3D,
};

enum class EImageUsageFlags
{
    None = 0,
    Sampled = 1,
    DepthStencil = 2,
    RenderTarget = 4,
};

DEFINE_ENUM_CLASS_BITWISE_OPERATORS(EImageUsageFlags);

template <typename TDerived>
class CImageBase : public tc::TLightRefBase<CImageBase<TDerived>>
{
protected:
    CImageBase();

public:
    virtual ~CImageBase() = default;
    void CopyFrom(const void* mem);
};

using CImage = CImageBase<TChooseImpl<CImageBase>::TDerived>;

} /* namespace RHI */
