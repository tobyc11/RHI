#include "ManagedPipeline.h"
#include "Device.h"

namespace RHI
{

CManagedPipeline::CManagedPipeline(CDevice& device, CPipelineDesc& desc)
{
    ReflectShaderModule(desc.VS);
    ReflectShaderModule(desc.PS);
    ReflectShaderModule(desc.GS);
    ReflectShaderModule(desc.DS);
    ReflectShaderModule(desc.HS);

    static const std::map<EPipelineResourceType, EDescriptorType> typeMap = {
        { EPipelineResourceType::SeparateSampler, EDescriptorType::Sampler },
        { EPipelineResourceType::CombinedImageSampler, EDescriptorType::Image },
        { EPipelineResourceType::SeparateImage, EDescriptorType::Image },
        { EPipelineResourceType::StorageImage, EDescriptorType::StorageImage },
        { EPipelineResourceType::UniformTexelBuffer, EDescriptorType::UniformTexelBuffer },
        { EPipelineResourceType::StorageTexelBuffer, EDescriptorType::StorageTexelBuffer },
        { EPipelineResourceType::UniformBuffer, EDescriptorType::UniformBuffer },
        { EPipelineResourceType::StorageBuffer, EDescriptorType::StorageBuffer },
    };

    // Some nonsense number that surely has no meaning
    uint32_t currSet = 0xF0F0F0F0;
    std::vector<CDescriptorSetLayoutBinding> bindings;
    for (const auto& pair : ResourceByBinding)
    {
        if (pair.first.first != currSet)
        {
            if (currSet != 0xF0F0F0F0 && !bindings.empty())
            {
                SetLayouts.resize(currSet + 1);
                SetLayouts[currSet] = device.CreateDescriptorSetLayout(bindings);
            }

            bindings.clear();
            currSet = pair.first.first;

            // In reality no body uses beyond 4 descriptor sets
            if (currSet > 1024)
                break;
        }

        CDescriptorSetLayoutBinding binding{};
        binding.Binding = pair.first.second;
        binding.Type = typeMap.at(pair.second.ResourceType);
        binding.StageFlags = pair.second.Stages;
        binding.Count = pair.second.ArraySize;
        bindings.emplace_back(binding);
    }
    if (!bindings.empty())
    {
        SetLayouts.resize(currSet + 1);
        SetLayouts[currSet] = device.CreateDescriptorSetLayout(bindings);
    }

    for (auto& layout : SetLayouts)
    {
        if (!layout)
        {
            layout = device.CreateDescriptorSetLayout({});
        }
    }
    PipelineLayout = device.CreatePipelineLayout(SetLayouts);

    desc.Layout = PipelineLayout;
    Pipeline = device.CreatePipeline(desc);

    ResourceByBinding.clear();
}

CDescriptorSet::Ref CManagedPipeline::CreateDescriptorSet(uint32_t set) const
{
    return SetLayouts[set]->CreateDescriptorSet();
}

std::vector<CDescriptorSet::Ref> CManagedPipeline::CreateDescriptorSets() const
{
    std::vector<CDescriptorSet::Ref> result;
    result.reserve(SetLayouts.size());
    for (const auto& layout : SetLayouts)
    {
        if (layout)
            result.push_back(layout->CreateDescriptorSet());
    }
    return result;
}

void CManagedPipeline::ReflectShaderModule(const CShaderModule::Ref& shaderModule)
{
    if (!shaderModule)
        return;

    // Grab all resources from each individual shader and put them into a big hash map
    for (const auto& resource : shaderModule->GetShaderResources())
    {
        auto key = std::make_pair(resource.Set, resource.Binding);
        if (resource.ResourceType == EPipelineResourceType::StageOutput
            || resource.ResourceType == EPipelineResourceType::StageInput)
            key = std::make_pair(-1 * static_cast<uint32_t>(resource.Stages), resource.Location);

        auto it = ResourceByBinding.find(key);
        if (it != ResourceByBinding.end())
            it->second.Stages |= resource.Stages;
        else
            ResourceByBinding.emplace(key, resource);
    }
}

}
