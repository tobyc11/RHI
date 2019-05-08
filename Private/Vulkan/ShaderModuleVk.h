#pragma once
#include "ShaderModule.h"
#include "VkCommon.h"

namespace RHI
{

class CShaderModuleVk : public CShaderModule
{
public:
    CShaderModuleVk(CDeviceVk& p, size_t size, const void* pCode);
    ~CShaderModuleVk() override;

    const std::string& GetEntryPoint() const { return EntryPoint; }
    VkShaderModule GetVkModule() const { return ShaderModule; }

    const std::vector<CPipelineResource>& GetShaderResources() const override { return Resources; }

private:
    CDeviceVk& Parent;
    VkShaderModule ShaderModule;
    VkShaderStageFlagBits Stage;
    std::string EntryPoint;
    std::vector<uint32_t> SPIRVBlob;
    std::vector<CPipelineResource> Resources;
};

} /* namespace RHI */
