#pragma once
#include "PipelineStateDesc.h"
#include "RenderPass.h"
#include "ShaderModule.h"
#include <LangUtils.h>

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
    uint32_t PatchControlPoints = 0;
    CRasterizerDesc RasterizerState;
    CMultisampleStateDesc MultisampleState;
    CDepthStencilDesc DepthStencilState;
    CBlendDesc BlendState;
    CRenderPass::WeakRef RenderPass;

    void VertexAttribFormat(uint32_t location, EFormat format, uint32_t offset, uint32_t binding)
    {
        CVertexInputAttributeDesc desc { location, format, offset, binding };
        VertexAttributes.push_back(desc);
    }

    void VertexBinding(uint32_t binding, bool bIsPerInstance = false)
    {
        CVertexInputBindingDesc desc { binding, bIsPerInstance };
        VertexBindings.push_back(desc);
    }
};

class CPipeline : public tc::FNonCopyable
{
public:
    typedef std::shared_ptr<CPipeline> Ref;

    virtual ~CPipeline() = default;
};

} /* namespace RHI */
