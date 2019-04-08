#include "Resources.h"
#ifdef RHI_IMPL_DIRECT3D11
#include "Direct3D11/BufferD3D11.h"
#include "Direct3D11/ImageD3D11.h"
#elif defined(RHI_IMPL_VULKAN)
#include "Vulkan/BufferVk.h"
#include "Vulkan/ImageVk.h"
#endif

namespace RHI
{

template <typename TDerived>
CBufferBase<TDerived>::CBufferBase(size_t size, EBufferUsageFlags usage)
    : Size(size)
    , Usage(usage)
{
}

template <typename TDerived> void* CBufferBase<TDerived>::Map(size_t offset, size_t size)
{
    return static_cast<TDerived*>(this)->Map(offset, size);
}

template <typename TDerived> void CBufferBase<TDerived>::Unmap()
{
    return static_cast<TDerived*>(this)->Unmap();
}

// Explicitly instanciate the wrapper for the chosen implementation
template class RHI_API CBufferBase<TChooseImpl<CBufferBase>::TDerived>;

template <typename TDerived> inline void CImageBase<TDerived>::CopyFrom(const void* mem)
{
    return static_cast<TDerived*>(this)->CopyFrom(mem);
}

template <typename TDerived> inline EFormat CImageBase<TDerived>::GetFormat() const
{
    return static_cast<const TDerived*>(this)->GetFormat();
}

template <typename TDerived> inline EImageUsageFlags CImageBase<TDerived>::GetUsageFlags() const
{
    return static_cast<const TDerived*>(this)->GetUsageFlags();
}

template <typename TDerived> inline uint32_t CImageBase<TDerived>::GetWidth() const
{
    return static_cast<const TDerived*>(this)->GetWidth();
}

template <typename TDerived> inline uint32_t CImageBase<TDerived>::GetHeight() const
{
    return static_cast<const TDerived*>(this)->GetHeight();
}

template <typename TDerived> inline uint32_t CImageBase<TDerived>::GetDepth() const
{
    return static_cast<const TDerived*>(this)->GetDepth();
}

template <typename TDerived> inline uint32_t CImageBase<TDerived>::GetMipLevels() const
{
    return static_cast<const TDerived*>(this)->GetMipLevels();
}

template <typename TDerived> inline uint32_t CImageBase<TDerived>::GetArrayLayers() const
{
    return static_cast<const TDerived*>(this)->GetArrayLayers();
}

template <typename TDerived> inline uint32_t CImageBase<TDerived>::GetSampleCount() const
{
    return static_cast<const TDerived*>(this)->GetSampleCount();
}

// Explicitly instanciate the wrapper for the chosen implementation
template class RHI_API CImageBase<TChooseImpl<CImageBase>::TDerived>;

} /* namespace RHI */
