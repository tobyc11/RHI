#pragma once
#include "RHIChooseImpl.h"
#include "Format.h"
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
    Sampled
};

template <typename TDerived>
class CImageBase : public tc::TLightRefBase<CImageBase<TDerived>>
{
protected:
    CImageBase();

public:
    virtual ~CImageBase() = default;
};

using CImage = CImageBase<TChooseImpl<CImageBase>::TDerived>;

} /* namespace Nome::RHI */
