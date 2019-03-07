#pragma once
#include "Format.h"
#include <RefBase.h>

namespace Nome::RHI
{

enum class EImageViewType
{
    View1D = 0,
    View2D = 1,
    View3D = 2,
    Cube = 3,
    View1DArray = 4,
    View2DArray = 5,
    CubeArray = 6,
};

struct CImageSubresourceRange
{
    uint32_t BaseMipLevel;
    uint32_t LevelCount;
    uint32_t BaseArrayLayer;
    uint32_t LayerCount;
};

struct CImageViewDesc
{
    EImageViewType Type;
    EFormat Format;
    CImageSubresourceRange Range;
};

class CImageView : public tc::TLightRefBase<CImageView>
{
public:
    virtual ~CImageView() = default;
};

} /* namespace Nome::RHI */
