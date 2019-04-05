#pragma once
#include "Resources.h"

namespace RHI
{

struct CSwapChainCreateInfo
{
    void* OSWindowHandle;
};

class CSwapChain
{
public:
    typedef std::shared_ptr<CSwapChain> Ref;

    virtual ~CSwapChain() = default;

    virtual CImage::Ref GetImage(uint32_t index) = 0;

    virtual void Resize(int width, int height) = 0;
    virtual void GetSize(int& width, int& height) const = 0;

    virtual void Present() = 0;
};

} /* namespace RHI */
