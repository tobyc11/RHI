#include "Buffer.h"
#ifdef RHI_IMPL_DIRECT3D11
#include "Direct3D11/BufferD3D11.h"
#endif

namespace RHI
{

template<typename TDerived>
CBufferBase<TDerived>::CBufferBase(uint32_t size, EBufferUsageFlags usage)
    : Size(size), Usage(usage)
{
}

template<typename TDerived>
void* CBufferBase<TDerived>::Map(size_t offset, size_t size)
{
    return static_cast<TDerived*>(this)->Map(offset, size);
}

template<typename TDerived>
void CBufferBase<TDerived>::Unmap()
{
    return static_cast<TDerived*>(this)->Unmap();
}

//Explicitly instanciate the wrapper for the chosen implementation
template class RHI_API CBufferBase<TChooseImpl<CBufferBase>::TDerived>;

} /* namespace RHI */
