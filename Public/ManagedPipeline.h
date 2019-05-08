#pragma once
#include "DescriptorSet.h"
#include "Pipeline.h"
#include "ShaderModule.h"
#include "RenderContext.h"
#include <array>
#include <map>

namespace RHI
{

// Basically the same thing as CPipeline, except you don't have to manually manage descriptor sets anymore
class CManagedPipeline
{
public:
    typedef std::shared_ptr<CManagedPipeline> Ref;

    CManagedPipeline(CDevice& device, CPipelineDesc& desc);

    CPipeline::Ref Get() const { return Pipeline; }

    CDescriptorSet::Ref CreateDescriptorSet(uint32_t set) const;
    std::vector<CDescriptorSet::Ref> CreateDescriptorSets() const;

private:
    void ReflectShaderModule(const CShaderModule::Ref& shaderModule);

    // Reflection data
    std::map<std::pair<uint32_t, uint32_t>, CPipelineResource> ResourceByBinding;

    std::vector<CDescriptorSetLayout::Ref> SetLayouts;
    CPipelineLayout::Ref PipelineLayout;
    CPipeline::Ref Pipeline;
};

}
