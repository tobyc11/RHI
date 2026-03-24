#pragma once
#include "MtlCommon.h"
#include "CopyContext.h"
#include "RenderPass.h"

#ifdef __OBJC__
@class MTLRenderPassDescriptor;
#endif

namespace RHI
{

class CRenderPassMetal : public CRenderPass
{
public:
    typedef std::shared_ptr<CRenderPassMetal> Ref;

    CRenderPassMetal(CDeviceMetal& parent, const CRenderPassDesc& desc);
    ~CRenderPassMetal() override = default;

    void SetSize(uint32_t width, uint32_t height) override;

#ifdef __OBJC__
    MTLRenderPassDescriptor* CreateMTLRenderPassDescriptor(
        uint32_t subpass, const std::vector<CClearValue>& clearValues) const;
#endif

    const CRenderPassDesc& GetDesc() const { return Desc; }
    uint32_t GetSubpassCount() const { return static_cast<uint32_t>(Desc.Subpasses.size()); }

private:
    CRenderPassDesc Desc;
};

} /* namespace RHI */
