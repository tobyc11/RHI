//
// Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#include "DescriptorSetLayoutVk.h"
#include "DescriptorPoolVk.h"
#include "DeviceVk.h"

#include <thread>

namespace RHI
{

CDescriptorSetLayoutVk::CDescriptorSetLayoutVk(CDeviceVk& p, const DescriptorSetLayoutHash& hash,
                                               const std::vector<CPipelineResource>& setResources)
    : Parent(p)
    , Hash(hash)
{
    // Static mapping between VulkanEZ pipeline resource types to Vulkan descriptor types.
    static const std::unordered_map<EPipelineResourceType, VkDescriptorType>
        resourceToDescriptorType = {
            { EPipelineResourceType::SeparateSampler, VK_DESCRIPTOR_TYPE_SAMPLER },
            { EPipelineResourceType::CombinedImageSampler,
              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER },
            { EPipelineResourceType::SeparateImage, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE },
            { EPipelineResourceType::StorageImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE },
            { EPipelineResourceType::UniformTexelBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER },
            { EPipelineResourceType::StorageTexelBuffer, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER },
            { EPipelineResourceType::UniformBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
            { EPipelineResourceType::StorageBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
            { EPipelineResourceType::SubpassInput, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT },
        };

    // Extract all unique resource types and their counts as well as layout bindings.
    for (const auto& b : setResources)
    {
        if (b.ResourceType == EPipelineResourceType::StageInput
            || b.ResourceType == EPipelineResourceType::StageOutput
            || b.ResourceType == EPipelineResourceType::PushConstantBuffer)
            break;

        VkDescriptorSetLayoutBinding bindingInfo;
        bindingInfo.binding = b.Binding;
        bindingInfo.descriptorCount = b.ArraySize;
        bindingInfo.descriptorType = resourceToDescriptorType.at(b.ResourceType);
        bindingInfo.stageFlags = b.Stages;
        bindingInfo.pImmutableSamplers = nullptr;
        Bindings.push_back(bindingInfo);

        BindingsLookup.emplace(b.Binding, bindingInfo);
    }

    // Create the Vulkan descriptor set layout handle.
    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = static_cast<uint32_t>(Bindings.size());
    layoutCreateInfo.pBindings = Bindings.data();
    auto result =
        vkCreateDescriptorSetLayout(Parent.GetVkDevice(), &layoutCreateInfo, nullptr, &Handle);
    if (result != VK_SUCCESS)
        throw CRHIRuntimeError("Vulkan descriptor set layout create failed");

    // Allocate a DescriptorPool from the new instance.
    DescriptorPool = new CDescriptorPoolVk(this);
}

CDescriptorSetLayoutVk::~CDescriptorSetLayoutVk()
{
    vkDestroyDescriptorSetLayout(Parent.GetVkDevice(), Handle, nullptr);
    delete DescriptorPool;
}

bool CDescriptorSetLayoutVk::GetLayoutBinding(uint32_t bindingIndex,
                                              VkDescriptorSetLayoutBinding** pBinding)
{
    auto it = BindingsLookup.find(bindingIndex);
    if (it == BindingsLookup.end())
        return false;

    *pBinding = &it->second;
    return true;
}

VkDescriptorSet CDescriptorSetLayoutVk::AllocateDescriptorSet()
{
    // Return new descriptor set allocation.
    return DescriptorPool->AllocateDescriptorSet();
}

VkResult CDescriptorSetLayoutVk::FreeDescriptorSet(VkDescriptorSet descriptorSet)
{
    // Free descriptor set handle.
    return DescriptorPool->FreeDescriptorSet(descriptorSet);
}
}