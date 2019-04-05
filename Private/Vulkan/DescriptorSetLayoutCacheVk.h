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
#include "ShaderModuleVk.h"
#include "VkCommon.h"

#include <map>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace RHI
{

typedef std::vector<uint32_t> DescriptorSetLayoutHash;

class CDescriptorSetLayoutVk;

class CDescriptorSetLayoutCacheVk
{
public:
    CDescriptorSetLayoutCacheVk(CDeviceVk& device);

    ~CDescriptorSetLayoutCacheVk();

    void CreateLayout(uint32_t setIndex, const std::vector<CPipelineResource>& setResources,
                      CDescriptorSetLayoutVk** pLayout);

    void DestroyLayout(CDescriptorSetLayoutVk* layout);

private:
    CDeviceVk& Parent;
    std::map<DescriptorSetLayoutHash, CDescriptorSetLayoutVk*> Layouts;
    std::unordered_map<CDescriptorSetLayoutVk*, uint32_t> LayoutReferences;
    std::mutex CacheMutex;
};

}