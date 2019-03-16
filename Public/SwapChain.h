#pragma once
#include "Image.h"

namespace RHI
{

using tc::sp;

struct CSwapChainCreateInfo
{
    void* OSWindowHandle;
};

class CSwapChain : public tc::CVirtualLightRefBase
{
public:
    virtual ~CSwapChain() = default;

    virtual sp<CImage> GetImage(uint32_t index) = 0;

    virtual void Resize(int width, int height) = 0;
    virtual void GetSize(int& width, int& height) const = 0;

    virtual void Present() = 0;
};

} /* namespace RHI */
