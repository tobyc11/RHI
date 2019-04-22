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
#pragma once
#include "VkCommon.h"
#include <SpinLock.h>
#include <map>
#include <queue>
#include <unordered_map>
#include <vector>

namespace RHI
{

class CDescriptorSetLayoutVk;

class CDescriptorPoolVk
{
public:
    CDescriptorPoolVk(CDescriptorSetLayoutVk* layout);

    ~CDescriptorPoolVk();

    VkDescriptorSet AllocateDescriptorSet();

    VkResult FreeDescriptorSet(VkDescriptorSet descriptorSet);

private:
    CDescriptorSetLayoutVk* Layout = nullptr;
    std::vector<VkDescriptorPoolSize> PoolSizes;
    std::vector<VkDescriptorPool> Pools;
    std::vector<uint32_t> AllocatedSets;
    uint32_t CurrentAllocationPoolIndex = 0;
    uint32_t MaxSetsPerPool = 50;

    std::unordered_map<VkDescriptorSet, uint32_t> AllocatedDescriptorSets;
    std::queue<VkDescriptorSet> RecycledDescriptorSets;
    tc::FSpinLock SpinLock;
};

}