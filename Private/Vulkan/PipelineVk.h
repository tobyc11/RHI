#pragma once
#include "DescriptorSetLayoutVk.h"
#include "Pipeline.h"
#include "ShaderModuleVk.h"
#include "VkCommon.h"

namespace RHI
{

class CPipelineVk : public CPipeline
{
public:
    CPipelineVk(CDeviceVk& p, const CPipelineDesc& desc);
    ~CPipelineVk() override;

    VkPipeline GetHandle() const { return PipelineHandle; }

private:
    void AddShaderModule(CShaderModule::Ref shaderModule, VkShaderStageFlagBits stage);

    CDeviceVk& Parent;

    std::vector<VkPipelineShaderStageCreateInfo> StageInfos;
    std::vector<std::string> EntryPoints;

    std::map<std::string, CPipelineResource> ResourceByName;
    std::unordered_map<uint32_t, std::vector<CPipelineResource>> SetBindings;
    std::unordered_map<uint32_t, CDescriptorSetLayoutVk*> SetLayouts;

    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkPipeline PipelineHandle = VK_NULL_HANDLE;
};

} /* namespace RHI */
