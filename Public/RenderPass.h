#pragma once
#include "Resources.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace RHI
{

enum class CAttachmentLoadOp : uint16_t
{
    Load,
    Clear,
    DontCare
};

enum class CAttachmentStoreOp : uint16_t
{
    Store,
    DontCare
};

struct CAttachmentDesc
{
    EFormat Format;
    uint32_t Samples;
    CAttachmentLoadOp LoadOp;
    CAttachmentStoreOp StoreOp;
    CAttachmentLoadOp StencilLoadOp;
    CAttachmentStoreOp StencilStoreOp;
    // Clear value is dynamically specified
    // Framebuffer is dynamically specified
};

struct CSubpassDesc
{
    static const uint32_t None = ~0U;

    std::vector<uint32_t> InputAttachments;
    std::vector<uint32_t> ColorAttachments;
    // uint32_t ResolveAttachment; TODO
    uint32_t DepthStencilAttachment;
    // std::vector<uint32_t> PreserveAttachments; TODO
    // Layouts are deduced from usage
};

struct CRenderPassDesc
{
    std::vector<CAttachmentDesc> Attachments;
    std::vector<CSubpassDesc> Subpasses;
    // Dependencies are deduced from usage
};

class CRenderPass
{
public:
    typedef std::shared_ptr<CRenderPass> Ref;

    virtual ~CRenderPass() = default;
};

struct CFramebufferDesc
{
    CRenderPass::Ref RenderPass;
    std::vector<CImageView::Ref> Attachments;
    uint32_t Width;
    uint32_t Height;
    uint32_t Layers;
};

class CFramebuffer
{
public:
    typedef std::shared_ptr<CFramebuffer> Ref;

    virtual ~CFramebuffer() = default;
};

}
