#pragma once
#include "Image.h"
#include "DrawTemplate.h"
#include "SwapChain.h"
#include <EnumClass.h>
#include <array>
#include <set>

namespace RHI
{

//We use an integer type to uniquely identify graph nodes and other sorts objects
//  __hash should be used to convert a string literal into an integer id.
using CNodeId = uint64_t;
const CNodeId kInvalidNodeId = 0;

class CRenderGraph;

enum ERenderTargetFlags
{
    Color = 1,
    DepthStencil = 2
};

DEFINE_ENUM_CLASS_BITWISE_OPERATORS(ERenderTargetFlags)

class CRenderTargetRef
{
public:
    //Make an undetermined render target
    CRenderTargetRef();
    //Render target from swapchain
    CRenderTargetRef(CSwapChain* swapChain);

    bool IsSwapChain() const { return SwapChain; }
    bool IsImportedImage() const { return false; }
    bool IsUndetermined() const { return !IsSwapChain() && !IsImportedImage(); }

    sp<CSwapChain> GetSwapChain() const { return SwapChain; }
    void GetDimensions(int& outWidth, int& outHeight);
    void SetDimensions(int width, int height);

    void SetImageViewFromSwapChain();
    void Reinit2D(int w, int h, ERenderTargetFlags flags);

    sp<CImageView> GetImageView() const { return ImageView; }

    void ClearImageAndView();

private:
    sp<CSwapChain> SwapChain;

    int Width, Height;

    sp<CImage> Image;
    sp<CImageView> ImageView;

    ERenderTargetFlags Flags;
};

class CRenderPass : public tc::TLightRefBase<CRenderPass>
{
    friend class CRenderGraph;

public:
    CRenderPass(CRenderGraph& renderGraph)
        : RenderGraph(renderGraph)
    {
    }

    //Rule of 5
    CRenderPass(const CRenderPass&) = delete;
    CRenderPass(CRenderPass&&) = delete;
    CRenderPass& operator=(const CRenderPass&) = delete;
    CRenderPass& operator=(CRenderPass&&) = delete;
    virtual ~CRenderPass() = default;

    CRenderGraph& GetRenderGraph() const { return RenderGraph; }

    void SetColorAttachment(uint32_t index, CNodeId id);
    void SetDepthStencilAttachment(CNodeId id);

    template <typename TLambda>
    void ForEachColorAttachment(const TLambda& lambda) const
    {
        for (int i = 0; i < 8; i++)
            lambda(ColorAttachments[i]);
    }

    CNodeId GetDepthStencilAttachment() const { return DepthStencilAttachment; }

    void AddDependency(CRenderPass& predecessor);
    void RemoveDependency(CRenderPass& predecessor);

private:
    virtual void Submit() const;

private:
    //Owner
    CRenderGraph& RenderGraph;

    //Attachments
    CNodeId ColorAttachments[8] = { 0 };
    CNodeId DepthStencilAttachment = kInvalidNodeId;

    //Graph adj data
    std::set<CRenderPass*> Pred;
    std::set<CRenderPass*> Succ;
};

class CDrawPassPriv;

class CDrawPass : public CRenderPass
{
public:
    CDrawPass(CRenderGraph& renderGraph);

    void BeginRecording();
    void Record(const CDrawTemplate& drawTemplate);
    void FinishRecording();
    void Submit() const override;

private:
    class Impl;
    std::unique_ptr<Impl> PImpl;
};

class CPresentPass : public CRenderPass
{
public:
    CPresentPass(CRenderGraph& renderGraph);

    void Submit() const override;
};

class CClearPass : public CRenderPass
{
public:
    CClearPass(CRenderGraph& renderGraph);

    void Submit() const override;

    std::array<float, 4> ClearColors[8];
    float ClearDepth = 1.0f;
    uint8_t ClearStencil = 0;

private:
    class Impl;
    std::unique_ptr<Impl> PImpl;
};

} /* namespace RHI */
