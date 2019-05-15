#pragma once
#include "BufferVk.h"
#include "ImageViewVk.h"
#include "VkCommon.h"
#include <map>
#include <unordered_map>

namespace RHI
{

// Modified from V-EZ
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

struct BindingInfo
{
    VkDeviceSize Offset;
    VkDeviceSize Range;
    VkBuffer BufferHandle = VK_NULL_HANDLE;

    CImageViewVk* ImageView = nullptr;
    VkAccessFlags ImageAccess;
    VkPipelineStageFlags ImageStages;
    VkImageLayout ImageLayout;

    VkSampler SamplerHandle = VK_NULL_HANDLE;

    BindingInfo() = default;

    BindingInfo(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range)
        : BufferHandle(buffer)
        , Offset(offset)
        , Range(range)
    {
    }

    BindingInfo(CImageViewVk* view, VkAccessFlags access, VkPipelineStageFlags stages,
                VkImageLayout layout)
        : ImageView(view)
        , ImageAccess(access)
        , ImageStages(stages)
        , ImageLayout(layout)
    {
    }

    BindingInfo(VkSampler sampler)
        : SamplerHandle(sampler)
    {
    }
};

typedef std::map<uint32_t, BindingInfo> ArrayBindings;

struct SetBindings
{
    std::unordered_map<uint32_t, ArrayBindings> Bindings;
    bool bDirty;
};

class CResourceBindings
{
public:
    bool IsDirty() const { return bDirty; }

    std::unordered_map<uint32_t, SetBindings>& GetSetBindings() { return BindingsBySet; }

    void ClearDirtyBit() { bDirty = false; }

    void Clear(uint32_t set);

    void Reset();

    void BindBuffer(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t set,
                    uint32_t binding, uint32_t arrayElement);
    void BindImageView(CImageViewVk* pImageView, VkAccessFlags access, VkPipelineStageFlags stages,
                       VkImageLayout layout, uint32_t set, uint32_t binding, uint32_t arrayElement);
    void BindSampler(VkSampler sampler, uint32_t set, uint32_t binding, uint32_t arrayElement);

private:
    void Bind(uint32_t set, uint32_t binding, uint32_t arrayElement, const BindingInfo& info);

    std::unordered_map<uint32_t, SetBindings> BindingsBySet;
    bool bDirty = false;
};

} /* namespace RHI */
