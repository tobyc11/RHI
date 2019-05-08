#include "DescriptorSetLayoutVk.h"
#include "DescriptorSetVk.h"
#include "DeviceVk.h"

#include <unordered_map>

namespace RHI
{

inline VkShaderStageFlags VkCast(EShaderStageFlags stageFlags)
{
    return static_cast<VkShaderStageFlags>(stageFlags);
}

CDescriptorSetLayoutVk::CDescriptorSetLayoutVk(CDeviceVk& p, const std::vector<CDescriptorSetLayoutBinding>& bindings)
    : Parent(p)
{
    static const std::unordered_map<EDescriptorType, VkDescriptorType>
        descriptorTypeMap = {
            { EDescriptorType::Sampler, VK_DESCRIPTOR_TYPE_SAMPLER },
            { EDescriptorType::Image, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE },
            { EDescriptorType::StorageImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE },
            { EDescriptorType::UniformTexelBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER },
            { EDescriptorType::StorageTexelBuffer, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER },
            { EDescriptorType::UniformBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
            { EDescriptorType::StorageBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
            { EDescriptorType::UniformBufferDynamic, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC },
            { EDescriptorType::StorageBufferDynamic, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC },
            { EDescriptorType::InputAttachment, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT },
        };

    for (const auto& b : bindings)
    {
        VkDescriptorSetLayoutBinding vkBinding;
        vkBinding.binding = b.Binding;
        vkBinding.descriptorCount = b.Count;
        vkBinding.descriptorType = descriptorTypeMap.at(b.Type);
        vkBinding.stageFlags = VkCast(b.StageFlags);
        vkBinding.pImmutableSamplers = nullptr;
        Bindings.push_back(vkBinding);
    }

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = static_cast<uint32_t>(Bindings.size());
    layoutCreateInfo.pBindings = Bindings.data();
    auto result =
        vkCreateDescriptorSetLayout(Parent.GetVkDevice(), &layoutCreateInfo, nullptr, &Handle);
    if (result != VK_SUCCESS)
        throw CRHIRuntimeError("Vulkan descriptor set layout create failed");
}

CDescriptorSetLayoutVk::~CDescriptorSetLayoutVk()
{
    vkDestroyDescriptorSetLayout(Parent.GetVkDevice(), Handle, nullptr);
}

CDescriptorSet::Ref CDescriptorSetLayoutVk::CreateDescriptorSet()
{
    return std::make_shared<CDescriptorSetVk>(std::static_pointer_cast<CDescriptorSetLayoutVk>(shared_from_this()));
}

const std::unique_ptr<CDescriptorPoolVk>& CDescriptorSetLayoutVk::GetDescriptorPool() const
{
    if (!Pool)
        Pool = std::make_unique<CDescriptorPoolVk>(this);
    return Pool;
}

CPipelineLayoutVk::CPipelineLayoutVk(CDeviceVk& p, const std::vector<CDescriptorSetLayout::Ref>& setLayouts)
    : Parent(p)
{
    assert(!setLayouts.empty());
    std::vector<VkDescriptorSetLayout> vkLayouts;
    for (const auto& it : setLayouts)
    {
        SetLayouts.emplace_back(std::static_pointer_cast<CDescriptorSetLayoutVk>(it));
        vkLayouts.emplace_back(SetLayouts.back()->GetHandle());
    }

    VkPipelineLayoutCreateInfo info = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    info.setLayoutCount = vkLayouts.size();
    info.pSetLayouts = vkLayouts.data();
    info.pushConstantRangeCount = 0;
    info.pPushConstantRanges = nullptr;
    vkCreatePipelineLayout(Parent.GetVkDevice(), &info, nullptr, &Handle);
}

CPipelineLayoutVk::~CPipelineLayoutVk()
{
    vkDestroyPipelineLayout(Parent.GetVkDevice(), Handle, nullptr);
}

}