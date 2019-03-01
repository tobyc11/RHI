#include "BufferD3D11.h"

namespace Nome::RHI
{

CBufferD3D11::CBufferD3D11(uint32_t size, EBufferUsageFlags usage, void* initialData)
    : CBufferBase(size, usage, initialData)
{
}

void* CBufferD3D11::Map(size_t offset, size_t size)
{
    return nullptr;
}

void CBufferD3D11::Unmap()
{
}

} /* namespace Nome::RHI */
