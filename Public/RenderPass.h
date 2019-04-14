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
    EAttachmentLoadOp StencilLoadOp = EAttachmentLoadOp::DontCare;
    EAttachmentStoreOp StencilStoreOp = EAttachmentStoreOp::DontCare;
    // Clear value is dynamically specified
    // Framebuffer is dynamically specified

    CAttachmentDesc(CImageView::Ref imageView, EAttachmentLoadOp l, EAttachmentStoreOp st)
        : ImageView(imageView)
        , LoadOp(l)
        , StoreOp(st)
    {
    }
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

    void AddInputAttachment(uint32_t index) { InputAttachments.push_back(index); }
    void AddColorAttachment(uint32_t index) { ColorAttachments.push_back(index); }
    void SetDepthStencilAttachment(uint32_t index) { DepthStencilAttachment = index; }
};

struct CRenderPassDesc
{
    std::vector<CAttachmentDesc> Attachments;
    std::vector<CSubpassDesc> Subpasses;
    uint32_t Width;
    uint32_t Height;
    uint32_t Layers;

    // Dependencies are deduced from usage
};

class CRenderPass
{
public:
    typedef std::shared_ptr<CRenderPass> Ref;

    virtual ~CRenderPass() = default;
};

}
