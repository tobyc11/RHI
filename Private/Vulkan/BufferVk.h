#pragma once
#include "Resources.h"
#include "VkCommon.h"

namespace RHI
{

class CBufferVk : public CBuffer
{
public:
    typedef std::shared_ptr<CBufferVk> Ref;

    CBufferVk(CDeviceVk& p, size_t size, EBufferUsageFlags usage, const void* initialData);
    ~CBufferVk() override;

    void* Map(size_t offset, size_t size);
    void Unmap();

    VkBuffer Buffer;
    VmaAllocation Allocation;

private:
    CDeviceVk& Parent;
};

} /* namespace RHI */
