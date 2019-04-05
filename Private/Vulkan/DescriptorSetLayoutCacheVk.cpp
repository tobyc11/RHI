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
#include "DescriptorSetLayoutCacheVk.h"
#include "DescriptorSetLayoutVk.h"

namespace RHI
{

static DescriptorSetLayoutHash GetHash(uint32_t setIndex,
                                       const std::vector<CPipelineResource>& setResources)
{
    // Descriptor set layout binding bit field declaration for generating hash.
    struct DescriptorSetLayoutBindingBitField
    {
        uint32_t descriptorCount : 32;
        uint16_t binding : 16;
        uint8_t descriptorType : 4;
        uint8_t stageFlags : 6;
    };

    // Generate bit field entries for each descriptor set resource.
    DescriptorSetLayoutHash hash(
        1 + setResources.size() * (sizeof(DescriptorSetLayoutBindingBitField) / 4));
    hash[0] = setIndex;
    DescriptorSetLayoutBindingBitField* bitfield =
        reinterpret_cast<DescriptorSetLayoutBindingBitField*>(&hash[1]);
    for (auto i = 0U; i < setResources.size(); ++i)
    {
        bitfield->binding = setResources[i].Binding;
        bitfield->descriptorType = static_cast<uint8_t>(setResources[i].ResourceType);
        bitfield->descriptorCount = setResources[i].ArraySize;
        bitfield->stageFlags = setResources[i].Stages;

        ++bitfield;
    }

    return hash;
}

CDescriptorSetLayoutCacheVk::CDescriptorSetLayoutCacheVk(CDeviceVk& device)
    : Parent(device)
{
}

CDescriptorSetLayoutCacheVk::~CDescriptorSetLayoutCacheVk()
{
    for (auto it : Layouts)
    {
        delete it.second;
    }
}

void CDescriptorSetLayoutCacheVk::CreateLayout(uint32_t setIndex,
                                               const std::vector<CPipelineResource>& setResources,
                                               CDescriptorSetLayoutVk** pLayout)
{
    // Generate hash from resource layout.
    auto hash = GetHash(setIndex, setResources);

    // Find or create a DescriptorSetLayout instance for the given descriptor set resouces.  Make
    // thread-safe.
    CDescriptorSetLayoutVk* descriptorSetLayout = nullptr;

    // Acquire access to the cache.
    std::unique_lock<std::mutex> lk(CacheMutex);

    // Look up hash.
    auto it = Layouts.find(hash);
    if (it != Layouts.end())
    {
        descriptorSetLayout = it->second;
        ++LayoutReferences[descriptorSetLayout];
    }
    else
    {
        descriptorSetLayout = new CDescriptorSetLayoutVk(Parent, hash, setResources);
        Layouts.emplace(std::move(hash), descriptorSetLayout);
        LayoutReferences.emplace(descriptorSetLayout, 1U);
    }

    *pLayout = descriptorSetLayout;
}

void CDescriptorSetLayoutCacheVk::DestroyLayout(CDescriptorSetLayoutVk* layout)
{
#if 0
        // Acquire access to the cache.
        m_spinLock.Lock();

        // Look up the layout's hash.
        const auto& hash = layout->GetHash();
        auto it = Layouts.find(hash);
        if (it != Layouts.end())
        {
            // Decrement reference count.
            auto& refCount = LayoutReferences.at(layout);
            if (--refCount == 0)
            {
                Layouts.erase(it);
                LayoutReferences.erase(layout);
                delete layout;
            }
        }

        // Release access to the cache.
        m_spinLock.Unlock();
#endif
}
}
