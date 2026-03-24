#include "PipelineMetal.h"
#include "DeviceMetal.h"
#include "ImageViewMetal.h"
#include "MtlHelpers.h"
#include "RenderPassMetal.h"
#include "ShaderModuleMetal.h"

namespace RHI
{

CPipelineMetal::CPipelineMetal(CDeviceMetal& parent, const CPipelineDesc& desc)
    : bIsCompute(false)
{
    MTLRenderPipelineDescriptor* rpd = [[MTLRenderPipelineDescriptor alloc] init];

    // Shaders
    if (desc.VS)
    {
        auto* vs = static_cast<CShaderModuleMetal*>(desc.VS.get());
        rpd.vertexFunction = vs->GetMTLFunction();
        VSRemap = vs->GetBindingRemap();
    }
    if (desc.PS)
    {
        auto* ps = static_cast<CShaderModuleMetal*>(desc.PS.get());
        rpd.fragmentFunction = ps->GetMTLFunction();
        PSRemap = ps->GetBindingRemap();
    }

    // Vertex descriptor
    if (!desc.VertexBindings.empty() || !desc.VertexAttributes.empty())
    {
        MTLVertexDescriptor* vd = [[MTLVertexDescriptor alloc] init];

        for (auto& attr : desc.VertexAttributes)
        {
            vd.attributes[attr.Location].format = MtlVertexFormat(attr.Format);
            vd.attributes[attr.Location].offset = attr.Offset;
            vd.attributes[attr.Location].bufferIndex = attr.Binding;
        }

        for (auto& bind : desc.VertexBindings)
        {
            vd.layouts[bind.Binding].stride = bind.Stride;
            vd.layouts[bind.Binding].stepRate = 1;
            vd.layouts[bind.Binding].stepFunction = bind.bIsPerInstance
                ? MTLVertexStepFunctionPerInstance
                : MTLVertexStepFunctionPerVertex;
        }

        rpd.vertexDescriptor = vd;
    }

    // Render pass attachment formats from the render pass desc
    auto renderPass = desc.RenderPass.lock();
    if (renderPass)
    {
        auto* mtlRP = static_cast<CRenderPassMetal*>(renderPass.get());
        const auto& rpDesc = mtlRP->GetDesc();

        if (desc.Subpass < rpDesc.Subpasses.size())
        {
            const auto& subpass = rpDesc.Subpasses[desc.Subpass];
            for (uint32_t i = 0; i < subpass.ColorAttachments.size(); ++i)
            {
                uint32_t attIdx = subpass.ColorAttachments[i];
                if (attIdx < rpDesc.Attachments.size() && rpDesc.Attachments[attIdx].ImageView)
                {
                    auto* view =
                        static_cast<CImageViewMetal*>(rpDesc.Attachments[attIdx].ImageView.get());
                    rpd.colorAttachments[i].pixelFormat =
                        MtlCast(view->GetDesc().Format);
                }
            }

            if (subpass.DepthStencilAttachment != CSubpassDesc::None)
            {
                uint32_t dsIdx = subpass.DepthStencilAttachment;
                if (dsIdx < rpDesc.Attachments.size() && rpDesc.Attachments[dsIdx].ImageView)
                {
                    auto* view =
                        static_cast<CImageViewMetal*>(rpDesc.Attachments[dsIdx].ImageView.get());
                    MTLPixelFormat dsPF = MtlCast(view->GetDesc().Format);
                    rpd.depthAttachmentPixelFormat = dsPF;
                    if (dsPF == MTLPixelFormatDepth32Float_Stencil8 ||
                        dsPF == MTLPixelFormatDepth24Unorm_Stencil8 ||
                        dsPF == MTLPixelFormatStencil8)
                        rpd.stencilAttachmentPixelFormat = dsPF;
                }
            }
        }
    }

    // Blend state
    for (uint32_t i = 0; i < 8; ++i)
    {
        const auto& rtBlend = desc.BlendState.RenderTargets[i];
        if (rpd.colorAttachments[i].pixelFormat == MTLPixelFormatInvalid)
            continue;

        rpd.colorAttachments[i].blendingEnabled = rtBlend.BlendEnable;
        rpd.colorAttachments[i].sourceRGBBlendFactor = MtlCast(rtBlend.SrcBlend);
        rpd.colorAttachments[i].destinationRGBBlendFactor = MtlCast(rtBlend.DestBlend);
        rpd.colorAttachments[i].rgbBlendOperation = MtlCast(rtBlend.BlendOp);
        rpd.colorAttachments[i].sourceAlphaBlendFactor = MtlCast(rtBlend.SrcBlendAlpha);
        rpd.colorAttachments[i].destinationAlphaBlendFactor = MtlCast(rtBlend.DestBlendAlpha);
        rpd.colorAttachments[i].alphaBlendOperation = MtlCast(rtBlend.BlendOpAlpha);
        rpd.colorAttachments[i].writeMask = MtlCast(rtBlend.RenderTargetWriteMask);
    }

    // Multisample
    if (desc.MultisampleState.MultisampleEnable)
    {
        rpd.rasterSampleCount = 4; // Default MSAA sample count
        rpd.alphaToCoverageEnabled = desc.MultisampleState.AlphaToCoverageEnable;
    }

    rpd.inputPrimitiveTopology = MtlTopologyClass(desc.PrimitiveTopology);

    NSError* error = nil;
    RenderPSO = [parent.GetMTLDevice() newRenderPipelineStateWithDescriptor:rpd error:&error];
    MTL_CHECK(error);

    // Depth stencil state
    MTLDepthStencilDescriptor* dsd = [[MTLDepthStencilDescriptor alloc] init];
    dsd.depthCompareFunction =
        desc.DepthStencilState.DepthEnable ? MtlCast(desc.DepthStencilState.DepthCompareOp)
                                           : MTLCompareFunctionAlways;
    dsd.depthWriteEnabled = desc.DepthStencilState.DepthWriteEnable;

    if (desc.DepthStencilState.StencilEnable)
    {
        MTLStencilDescriptor* front = [[MTLStencilDescriptor alloc] init];
        front.stencilFailureOperation = MtlCast(desc.DepthStencilState.Front.FailOp);
        front.depthStencilPassOperation = MtlCast(desc.DepthStencilState.Front.PassOp);
        front.depthFailureOperation = MtlCast(desc.DepthStencilState.Front.DepthFailOp);
        front.stencilCompareFunction = MtlCast(desc.DepthStencilState.Front.CompareOp);
        front.readMask = desc.DepthStencilState.Front.CompareMask;
        front.writeMask = desc.DepthStencilState.Front.WriteMask;
        dsd.frontFaceStencil = front;

        MTLStencilDescriptor* back = [[MTLStencilDescriptor alloc] init];
        back.stencilFailureOperation = MtlCast(desc.DepthStencilState.Back.FailOp);
        back.depthStencilPassOperation = MtlCast(desc.DepthStencilState.Back.PassOp);
        back.depthFailureOperation = MtlCast(desc.DepthStencilState.Back.DepthFailOp);
        back.stencilCompareFunction = MtlCast(desc.DepthStencilState.Back.CompareOp);
        back.readMask = desc.DepthStencilState.Back.CompareMask;
        back.writeMask = desc.DepthStencilState.Back.WriteMask;
        dsd.backFaceStencil = back;
    }

    DepthStencilState = [parent.GetMTLDevice() newDepthStencilStateWithDescriptor:dsd];

    // Rasterizer state (stored for encoder setup)
    CullMode = MtlCast(desc.RasterizerState.CullMode);
    FillMode = MtlCast(desc.RasterizerState.PolygonMode);
    Winding = MtlWinding(desc.RasterizerState.FrontFaceCCW);
    DepthBiasEnable = desc.RasterizerState.DepthBiasEnable;
    DepthBiasConstant = desc.RasterizerState.DepthBiasConstantFactor;
    DepthBiasSlope = desc.RasterizerState.DepthBiasSlopeFactor;
    DepthBiasClamp = desc.RasterizerState.DepthBiasClamp;
    PrimitiveType = MtlCast(desc.PrimitiveTopology);
}

CPipelineMetal::CPipelineMetal(CDeviceMetal& parent, const CComputePipelineDesc& desc)
    : bIsCompute(true)
{
    auto* cs = static_cast<CShaderModuleMetal*>(desc.CS.get());
    CSRemap = cs->GetBindingRemap();

    NSError* error = nil;
    ComputePSO = [parent.GetMTLDevice()
        newComputePipelineStateWithFunction:cs->GetMTLFunction()
                                     error:&error];
    MTL_CHECK(error);
}

CPipelineMetal::~CPipelineMetal() { }

} /* namespace RHI */
