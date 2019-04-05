#pragma once
#include <cstdint>
#include <memory>

namespace RHI
{

struct COffset2D
{
    int32_t X;
    int32_t Y;
};

struct COffset3D
{
    int32_t X;
    int32_t Y;
    int32_t Z;
};

struct CExtent2D
{
    uint32_t Width;
    uint32_t Height;
};

struct CExtent3D
{
    uint32_t Width;
    uint32_t Height;
    uint32_t Depth;
};

struct CRect2D
{
    COffset2D Offset;
    CExtent2D Extent;

    CRect2D& SetX(int32_t value)
    {
        Offset.X = value;
        return *this;
    }
    CRect2D& SetY(int32_t value)
    {
        Offset.Y = value;
        return *this;
    }
    CRect2D& SetWidth(uint32_t value)
    {
        Extent.Width = value;
        return *this;
    }
    CRect2D& SetHeight(uint32_t value)
    {
        Extent.Height = value;
        return *this;
    }
};

enum class EResourceState : uint32_t
{
    Undefined,
    PreInitialized,
    Common,
    VertexBuffer,
    ConstantBuffer,
    IndexBuffer,
    RenderTarget,
    UnorderedAccess,
    DepthStencil,
    ShaderResource,
    StreamOut,
    IndirectArg,
    CopyDest,
    CopySource,
    ResolveDest,
    ResolveSource,
    Present,
    GenericRead,
    Predication,
    NonPixelShader
};

} /* namespace RHI */
