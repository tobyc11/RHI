#pragma once
#include "PipelineStateDesc.h"
#include "RenderPass.h"
#include "ShaderModule.h"

namespace RHI
{

// TODO make arguments weak references. It's awkward if a desc holds a strong reference and we can't
// release that object in time
struct CPipelineDesc
{
    CShaderModule::Ref VS;
    CShaderModule::Ref PS;
    CShaderModule::Ref GS;
    CShaderModule::Ref HS;
    CShaderModule::Ref DS;
    std::vector<CVertexInputAttributeDesc> VertexAttributes;
    std::vector<CVertexInputBindingDesc> VertexBindings;
    EPrimitiveTopology PrimitiveTopology = EPrimitiveTopology::TriangleList;
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
