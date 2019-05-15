#pragma once
#include "DescriptorPoolVk.h"
#include "DescriptorSet.h"
#include "VkCommon.h"
#include "VkHelpers.h"

#include <thread>
#include <unordered_map>
#include <vector>

namespace RHI
{

class CDescriptorSetLayoutVk : public CDescriptorSetLayout
{
public:
    typedef std::shared_ptr<CDescriptorSetLayoutVk> Ref;

    CDescriptorSetLayoutVk(CDeviceVk& p, const std::vector<CDescriptorSetLayoutBinding>& bindings);
    ~CDescriptorSetLayoutVk() override;

    // Allocate and create a descriptor set from this layout
    CDescriptorSet::Ref CreateDescriptorSet() override;

    CDeviceVk& GetDevice() const { return Parent; }
    VkDescriptorSetLayout GetHandle() const { return Handle; }
    const std::vector<VkDescriptorSetLayoutBinding>& GetBindings() const { return Bindings; }
    VkDescriptorType GetDescriptorType(uint32_t binding) const { return BindingToType.at(binding); }
    VkPipelineStageFlags GetPipelineStages(uint32_t binding) const
    {
        return BindingToStages.at(binding);
    }

    const std::unique_ptr<CDescriptorPoolVk>& GetDescriptorPool() const;

private:
    CDeviceVk& Parent;
    VkDescriptorSetLayout Handle = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayoutBinding> Bindings;
    std::map<uint32_t, VkDescriptorType> BindingToType;
    std::map<uint32_t, VkPipelineStageFlags> BindingToStages;

    mutable std::unique_ptr<CDescriptorPoolVk> Pool;
};

class CPipelineLayoutVk : public CPipelineLayout
{
public:
    typedef std::shared_ptr<CPipelineLayoutVk> Ref;

    CPipelineLayoutVk(CDeviceVk& p, const std::vector<CDescriptorSetLayout::Ref>& setLayouts);
    ~CPipelineLayoutVk() override;

    CDeviceVk& GetDevice() const { return Parent; }
    VkPipelineLayout GetHandle() const { return Handle; }
    const std::vector<CDescriptorSetLayoutVk::Ref>& GetSetLayouts() const { return SetLayouts; }

private:
    CDeviceVk& Parent;
    std::vector<CDescriptorSetLayoutVk::Ref> SetLayouts;

    VkPipelineLayout Handle;
};

}