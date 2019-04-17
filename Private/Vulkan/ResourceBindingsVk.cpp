#include "ResourceBindingsVk.h"

namespace RHI
{

void ResourceBindings::Clear(uint32_t set) { BindingsBySet.erase(set); }

void ResourceBindings::Reset()
{
    BindingsBySet.clear();
    bDirty = false;
}

void ResourceBindings::BindBuffer(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range,
                                  uint32_t set, uint32_t binding, uint32_t arrayElement)
{
    Bind(set, binding, arrayElement,
         BindingInfo { offset, range, buffer, VK_NULL_HANDLE, VK_NULL_HANDLE });
}

void ResourceBindings::BindImageView(CImageViewVk* pImageView, VkSampler sampler, uint32_t set,
                                     uint32_t binding, uint32_t arrayElement)
{
    Bind(set, binding, arrayElement, BindingInfo { 0, 0, VK_NULL_HANDLE, pImageView, sampler });
}

void ResourceBindings::BindSampler(VkSampler sampler, uint32_t set, uint32_t binding,
                                   uint32_t arrayElement)
{
    Bind(set, binding, arrayElement, BindingInfo { 0, 0, VK_NULL_HANDLE, VK_NULL_HANDLE, sampler });
}

void ResourceBindings::Bind(uint32_t set, uint32_t binding, uint32_t arrayElement,
                            const BindingInfo& info)
{
    // If resource is being removed from binding, erase the entry.
    if (info.BufferHandle == VK_NULL_HANDLE && info.pImageView == VK_NULL_HANDLE
        && info.sampler == VK_NULL_HANDLE)
    {
        auto it = BindingsBySet.find(set);
        if (it != BindingsBySet.end())
        {
            auto& setBindings = it->second;
            auto it2 = setBindings.Bindings.find(binding);
            if (it2 != setBindings.Bindings.end())
            {
                auto& arrayBindings = it2->second;
                auto it3 = arrayBindings.find(arrayElement);
                if (it3 != arrayBindings.end())
                {
                    arrayBindings.erase(it3);

                    if (arrayBindings.size() == 0)
                        setBindings.Bindings.erase(it2);

                    setBindings.bDirty = true;
                }
            }
        }
    }
    // Else binding is being added.
    else
    {
        // If set # does not exist yet, add it.
        auto it = BindingsBySet.find(set);
        if (it == BindingsBySet.end())
        {
            ArrayBindings arrayBinding = { { arrayElement, info } };

            SetBindings setBindings;
            setBindings.Bindings.emplace(binding, std::move(arrayBinding));
            setBindings.bDirty = true;

            BindingsBySet.emplace(set, std::move(setBindings));
        }
        // Else, find the binding #.
        else
        {
            // If the binding # does not exist, create it and add the array element.
            auto& setBinding = it->second;
            auto it2 = setBinding.Bindings.find(binding);
            if (it2 == setBinding.Bindings.end())
            {
                ArrayBindings arrayBinding = { { arrayElement, info } };
                setBinding.Bindings.emplace(binding, std::move(arrayBinding));
            }
            else
            {
                it2->second[arrayElement] = info;
                setBinding.bDirty = true;
            }
        }
    }

    // Always mark ResourceBindings as dirty for fast checking during descriptor set binding.
    bDirty = true;
}

}
