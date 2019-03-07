#pragma once
#include "RHIChooseImpl.h"
#include "Format.h"
#include <EnumClass.h>
#include <RefBase.h>

namespace Nome::RHI
{

enum class EImageType
{
    Image1D,
    Image2D,
    Image3D,
};

enum class EImageUsageFlags
{
    Sampled,
    DepthStencil
};

DEFINE_ENUM_CLASS_BITWISE_OPERATORS(EImageUsageFlags);

template <typename TDerived>
class CImageBase : public tc::TLightRefBase<CImageBase<TDerived>>
{
protected:
    CImageBase();

public:
    virtual ~CImageBase() = default;
    void CopyFrom(void* mem);
};

using CImage = CImageBase<TChooseImpl<CImageBase>::TDerived>;

} /* namespace Nome::RHI */
