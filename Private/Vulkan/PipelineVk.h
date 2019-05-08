#pragma once
#include "DescriptorSetLayoutVk.h"
#include "Pipeline.h"
#include "ShaderModuleVk.h"
#include "VkCommon.h"
#include <set>

namespace RHI
{

const size_t kMaxBoundDescriptorSets = 32;

class CPipelineVk : public CPipeline
{
public:
    CPipelineVk(CDeviceVk& p, const CPipelineDesc& desc);
    ~CPipelineVk() override;

    VkPipeline GetHandle() const { return PipelineHandle; }

    VkPipelineLayout GetPipelineLayout() const;

private:
    void AddShaderModule(const CShaderModule::Ref& shaderModule, VkShaderStageFlagBits stage);

    CDeviceVk& Parent;

    std::vector<VkPipelineShaderStageCreateInfo> StageInfos;
    std::vector<std::string> EntryPoints;

    CPipelineLayoutVk::Ref PipelineLayout;
    VkPipeline PipelineHandle = VK_NULL_HANDLE;
};

} /* namespace RHI */
