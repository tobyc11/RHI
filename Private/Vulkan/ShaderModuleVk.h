#pragma once
#include "ShaderModule.h"
#include "VkCommon.h"

namespace RHI
{

// The following 3 definitions are used for spriv shader reflection
enum class EPipelineResourceType : uint8_t
{
    StageInput,
    StageOutput,
    SeparateSampler,
    CombinedImageSampler,
    SeparateImage,
    StorageImage,
    UniformTexelBuffer,
    StorageTexelBuffer,
    UniformBuffer,
    StorageBuffer,
    SubpassInput,
    PushConstantBuffer,
};

enum class EBaseType
{
    Bool,
    Char,
    Int,
    UInt,
    Half,
    Float,
    Double,
    Struct
};

struct CPipelineResource
{
    VkShaderStageFlags Stages;
    EPipelineResourceType ResourceType;
    EBaseType BaseType;
    VkAccessFlags Access;
    uint32_t Set;
    uint32_t Binding;
    uint32_t Location;
    uint32_t InputAttachmentIndex;
    uint32_t VecSize;
    uint32_t Columns;
    uint32_t ArraySize;
    uint32_t Offset;
    uint32_t Size;
    char Name[VK_MAX_DESCRIPTION_SIZE];
};

class CShaderModuleVk : public CShaderModule
{
public:
    CShaderModuleVk(CDeviceVk& p, size_t size, const void* pCode);
    ~CShaderModuleVk() override;

    const std::string& GetEntryPoint() const { return EntryPoint; }
    VkShaderModule GetVkModule() const { return ShaderModule; }
    const std::vector<CPipelineResource>& GetShaderResources() const { return Resources; }

private:
    CDeviceVk& Parent;
    VkShaderModule ShaderModule;
    VkShaderStageFlagBits Stage;
    std::string EntryPoint;
    std::vector<uint32_t> SPIRVBlob;
    std::vector<CPipelineResource> Resources;
};

} /* namespace RHI */
