#pragma once
#include <RefBase.h>

namespace Nome::RHI
{

struct CSwapChainCreateInfo
{
    void* OSWindowHandle;
};

class CSwapChain : public tc::CVirtualLightRefBase
{
public:
    virtual ~CSwapChain() = default;

    virtual void Resize(int width, int height) = 0;
};

} /* namespace Nome::RHI */
