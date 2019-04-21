#pragma once
#include "Resources.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace RHI
{

enum class EAttachmentLoadOp : uint16_t
{
    Load,
    Clear,
    DontCare
};

enum class EAttachmentStoreOp : uint16_t
{
    Store,
    DontCare
};

struct CAttachmentDesc
{
    CImageView::Ref ImageView;
    EAttachmentLoadOp LoadOp;
    EAttachmentStoreOp StoreOp;
    EAttachmentLoadOp StencilLoadOp;
    EAttachmentStoreOp StencilStoreOp;
};

struct CSubpassDesc
{
    static const uint32_t None = ~0U;

    std::vector<uint32_t> InputAttachments;
    std::vector<uint32_t> ColorAttachments;
    // uint32_t ResolveAttachment; TODO
    uint32_t DepthStencilAttachment = None;
    // std::vector<uint32_t> PreserveAttachments; TODO
    // Layouts are deduced from usage

    CSubpassDesc& AddInputAttachment(uint32_t index)
    {
        InputAttachments.push_back(index);
        return *this;
    }

    CSubpassDesc& AddColorAttachment(uint32_t index)
    {
        ColorAttachments.push_back(index);
        return *this;
    }

    CSubpassDesc& SetDepthStencilAttachment(uint32_t index)
    {
        DepthStencilAttachment = index;
        return *this;
    }
};

struct CRenderPassDesc
{
    std::vector<CAttachmentDesc> Attachments;
    std::vector<CSubpassDesc> Subpasses;
    uint32_t Width;
    uint32_t Height;
    uint32_t Layers;

    void AddAttachment(CImageView::Ref imageView, EAttachmentLoadOp loadOp,
                       EAttachmentStoreOp storeOp,
                       EAttachmentLoadOp stencilLoadOp = EAttachmentLoadOp::DontCare,
                       EAttachmentStoreOp stencilStoreOp = EAttachmentStoreOp::DontCare)
    {
        CAttachmentDesc desc;
        desc.ImageView = imageView;
        desc.LoadOp = loadOp;
        desc.StoreOp = storeOp;
        desc.StencilLoadOp = stencilLoadOp;
        desc.StencilStoreOp = stencilStoreOp;
        Attachments.push_back(std::move(desc));
    }

    CSubpassDesc& NextSubpass()
    {
        Subpasses.emplace_back();
        return Subpasses.back();
    }

    void SetExtent(uint32_t width, uint32_t height, uint32_t layers = 1)
    {
        Width = width;
        Height = height;
        Layers = layers;
    }
};

class CRenderPass : public tc::FNonCopyable
{
public:
    typedef std::shared_ptr<CRenderPass> Ref;
    typedef std::weak_ptr<CRenderPass> WeakRef;

    virtual ~CRenderPass() = default;
};

}
