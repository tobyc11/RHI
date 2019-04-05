#pragma once
#include "Format.h"
#include "RHIChooseImpl.h"
#include <EnumClass.h>
#include <cstdint>
#include <memory>

namespace RHI
{

// Buffer

enum class EBufferUsageFlags
{
    VertexBuffer = 1,
    IndexBuffer = 2,
    ConstantBuffer = 4,
    Streaming = 8,
};

DEFINE_ENUM_CLASS_BITWISE_OPERATORS(EBufferUsageFlags)

template <typename TDerived>
class CBufferBase : public std::enable_shared_from_this<CBufferBase<TDerived>>
{
protected:
    CBufferBase(uint32_t size, EBufferUsageFlags usage);

public:
    typedef std::shared_ptr<CBufferBase> Ref;

    virtual ~CBufferBase() = default;

    void* Map(size_t offset, size_t size);
    void Unmap();

protected:
    uint32_t Size;
    EBufferUsageFlags Usage;
};

class CBufferView
{
public:
    typedef std::shared_ptr<CBufferView> Ref;

    virtual ~CBufferView() = default;
};

// Image

enum class EImageType
{
    Image1D,
    Image2D,
    Image3D,
};

enum class EImageUsageFlags
{
    None = 0,
    Sampled = 1 << 0,
    DepthStencil = 1 << 1,
    RenderTarget = 1 << 2,
    CubeMap = 1 << 3,
    GenMIPMaps = 1 << 4,
    Staging = 1 << 5,
};

DEFINE_ENUM_CLASS_BITWISE_OPERATORS(EImageUsageFlags);

template <typename TDerived>
class CImageBase : public std::enable_shared_from_this<CImageBase<TDerived>>
{
public:
    typedef std::shared_ptr<CImageBase> Ref;

    virtual ~CImageBase() = default;

    void CopyFrom(const void* mem);
    EFormat GetFormat() const;
    EImageUsageFlags GetUsageFlags() const;
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    uint32_t GetDepth() const;
    uint32_t GetMipLevels() const;
    uint32_t GetArrayLayers() const;
    uint32_t GetSampleCount() const;

protected:
    CImageBase() = default;
};

// Image View

enum class EImageViewType
{
    View1D = 0,
    View2D = 1,
    View3D = 2,
    Cube = 3,
    View1DArray = 4,
    View2DArray = 5,
    CubeArray = 6,
};

struct CImageSubresourceRange
{
    uint32_t BaseMipLevel = 0;
    uint32_t LevelCount = 1;
    uint32_t BaseArrayLayer = 0;
    uint32_t LayerCount = 1;
};

struct CImageViewDesc
{
    EImageViewType Type;
    EFormat Format;
    CImageSubresourceRange Range;
};

class CImageView
{
public:
    typedef std::shared_ptr<CImageView> Ref;

    virtual ~CImageView() = default;
};

} /* namespace RHI */
