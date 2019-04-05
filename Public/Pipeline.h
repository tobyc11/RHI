#pragma once
#include "PipelineStateDesc.h"
#include "RenderPass.h"
#include "ShaderModule.h"

namespace RHI
{

struct CPipelineDesc
{
    CShaderModule::Ref VS;
    CShaderModule::Ref PS;
    CShaderModule::Ref GS;
    CShaderModule::Ref HS;
    CShaderModule::Ref DS;
    std::vector<CVertexInputAttributeDesc> VertexAttributes;
    std::vector<CVertexInputBindingDesc> VertexBindings;
    EPrimitiveTopology PrimitiveTopology;
    uint32_t PatchControlPoints;
    const CRasterizerDesc* RasterizerState;
    const CMultisampleStateDesc* MultisampleState;
    const CDepthStencilDesc* DepthStencilState;
    const CBlendDesc* BlendState;
    CRenderPass::Ref RenderPass;
};

class CPipeline
{
public:
    typedef std::shared_ptr<CPipeline> Ref;

    virtual ~CPipeline() = default;
};

} /* namespace RHI */
