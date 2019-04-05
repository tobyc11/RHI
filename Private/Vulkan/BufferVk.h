#pragma once
#include "Resources.h"
#include "VkCommon.h"

namespace RHI
{

class CBufferVk : public CBuffer
{
public:
    CBufferVk(CDeviceVk& p, size_t size, EBufferUsageFlags usage, const void* initialData);

    ~CBufferVk();

    VkBuffer Buffer;
    VmaAllocation Allocation;

private:
    CDeviceVk& Parent;
};

} /* namespace RHI */
