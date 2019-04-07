#pragma once
#include "Resources.h"

namespace RHI
{

struct CSwapChainPresentInfo
{
    CImageView::Ref SrcImage;
};

class CSwapChain : public std::enable_shared_from_this<CSwapChain>
{
public:
    typedef std::shared_ptr<CSwapChain> Ref;
    typedef std::weak_ptr<CSwapChain> WeakRef;

    virtual ~CSwapChain() = default;

    virtual void Resize(uint32_t width, uint32_t height) = 0;
    virtual void GetSize(uint32_t& width, uint32_t& height) const = 0;

	virtual CImage::Ref GetImage() = 0;

    virtual void AcquireNextImage() = 0;
    virtual void Present(const CSwapChainPresentInfo& info) = 0;
};

} /* namespace RHI */
