#pragma once
#include <RefBase.h>

namespace Nome::RHI
{

class CImageView : public tc::TLightRefBase<CImageView>
{
public:
    virtual ~CImageView() = default;
};

} /* namespace Nome::RHI */
