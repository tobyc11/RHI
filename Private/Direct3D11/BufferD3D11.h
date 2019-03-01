#pragma once
#include "Buffer.h"

namespace Nome::RHI
{

class CBufferD3D11 : public CBufferBase<CBufferD3D11>
{
public:
    CBufferD3D11(uint32_t size, EBufferUsageFlags usage, void* initialData = nullptr);

    void* Map(size_t offset, size_t size);
    void Unmap();
};

} /* namespace Nome::RHI */
