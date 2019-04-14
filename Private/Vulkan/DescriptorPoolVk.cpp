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
#include "DescriptorPoolVk.h"
#include "DescriptorSetLayoutVk.h"
#include "DeviceVk.h"

namespace RHI
{

CDescriptorPoolVk::CDescriptorPoolVk(CDescriptorSetLayoutVk* layout)
    : Layout(layout)
{
    // Get the layout's binding information.
    const auto& bindings = layout->GetBindings();

    // Generate array of pool sizes for each unique resource type.
    std::unordered_map<VkDescriptorType, uint32_t> descriptorTypeCounts;
    for (auto& binding : bindings)
    {
        descriptorTypeCounts[binding.descriptorType] += binding.descriptorCount;
    }

    // Fill in pool sizes array.
    PoolSizes.resize(descriptorTypeCounts.size());
    uint32_t index = 0;
    for (auto& it : descriptorTypeCounts)
    {
        PoolSizes[index].type = it.first;
        PoolSizes[index].descriptorCount = it.second * MaxSetsPerPool;
        ++index;
    }
}

CDescriptorPoolVk::~CDescriptorPoolVk()
{
    // Destroy all allocated descriptor sets.
    for (auto it : AllocatedDescriptorSets)
    {
        vkFreeDescriptorSets(Layout->GetDevice().GetVkDevice(), Pools[it.second], 1, &it.first);
    }

    // Destroy all created pools.
    for (auto pool : Pools)
    {
        vkDestroyDescriptorPool(Layout->GetDevice().GetVkDevice(), pool, nullptr);
    }
}

VkDescriptorSet CDescriptorPoolVk::AllocateDescriptorSet()
{
    // Safe guard access to internal resources across threads.
    SpinLock.Lock();

    // Find the next pool to allocate from.
    while (true)
    {
        // Allocate a new VkDescriptorPool if necessary.
        if (Pools.size() <= CurrentAllocationPoolIndex)
        {
            // Create the Vulkan descriptor pool.
            VkDescriptorPoolCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            createInfo.poolSizeCount = static_cast<uint32_t>(PoolSizes.size());
            createInfo.pPoolSizes = PoolSizes.data();
            createInfo.maxSets = MaxSetsPerPool;
            VkDescriptorPool handle = VK_NULL_HANDLE;
            auto result = vkCreateDescriptorPool(Layout->GetDevice().GetVkDevice(), &createInfo,
                                                 nullptr, &handle);
            if (result != VK_SUCCESS)
                return VK_NULL_HANDLE;

            // Add the Vulkan handle to the descriptor pool instance.
            Pools.push_back(handle);
            AllocatedSets.push_back(0);
            break;
        }
        else if (AllocatedSets[CurrentAllocationPoolIndex] < MaxSetsPerPool)
            break;

        // Increment pool index.
        ++CurrentAllocationPoolIndex;
    }

    // Increment allocated set count for given pool.
    ++AllocatedSets[CurrentAllocationPoolIndex];

    // Allocate a new descriptor set from the current pool index.
    VkDescriptorSetLayout setLayout = Layout->GetHandle();

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = Pools[CurrentAllocationPoolIndex];
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &setLayout;
    VkDescriptorSet handle = VK_NULL_HANDLE;
    auto result = vkAllocateDescriptorSets(Layout->GetDevice().GetVkDevice(), &allocInfo, &handle);
    if (result != VK_SUCCESS)
        return VK_NULL_HANDLE;

    // Store an internal mapping between the descriptor set handle and it's parent pool.
    // This is used when FreeDescriptorSet is called downstream.
    AllocatedDescriptorSets.emplace(handle, CurrentAllocationPoolIndex);

    SpinLock.Unlock();
    // Return descriptor set handle.
    return handle;
}

VkResult CDescriptorPoolVk::FreeDescriptorSet(VkDescriptorSet descriptorSet)
{
    // Safe guard access to internal resources across threads.
    SpinLock.Lock();

    // Get the index of the descriptor pool the descriptor set was allocated from.
    auto it = AllocatedDescriptorSets.find(descriptorSet);
    if (it == AllocatedDescriptorSets.end())
        return VK_INCOMPLETE;

    // Return the descriptor set to the original pool.
    auto poolIndex = it->second;
    vkFreeDescriptorSets(Layout->GetDevice().GetVkDevice(), Pools[poolIndex], 1, &descriptorSet);

    // Remove descriptor set from allocatedDescriptorSets map.
    AllocatedDescriptorSets.erase(descriptorSet);

    // Decrement the number of allocated descriptor sets for the pool.
    --AllocatedSets[poolIndex];

    // Set the next allocation to use this pool index.
    CurrentAllocationPoolIndex = std::min(CurrentAllocationPoolIndex, poolIndex);

    SpinLock.Unlock();
    // Return success.
    return VK_SUCCESS;
}

}