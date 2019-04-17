#pragma once
#include "RHIModuleAPI.h"
#include <cstdint>
#include <memory>

namespace RHI
{

struct COffset2D
{
    int32_t X;
    int32_t Y;

    void Set(int32_t x, int32_t y)
    {
        X = x;
        Y = y;
    }
};

struct COffset3D
{
    int32_t X;
    int32_t Y;
    int32_t Z;

    void Set(int32_t x, int32_t y, int32_t z)
    {
        X = x;
        Y = y;
        Z = z;
    }
};

struct CExtent2D
{
    uint32_t Width;
    uint32_t Height;

    void Set(uint32_t w, uint32_t h)
    {
        Width = w;
        Height = h;
    }
};

struct CExtent3D
{
    uint32_t Width;
    uint32_t Height;
    uint32_t Depth;

    void Set(uint32_t w, uint32_t h, uint32_t d)
    {
        Width = w;
        Height = h;
        Depth = d;
    }
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

// Signify the GPU's access to a resource: the when, how, and format
enum class EResourceState : uint32_t
{
    Undefined,
    PreInitialized,
    General,
    IndirectArg,
    IndexBuffer,
    VertexBuffer,
    ConstantBuffer,
    RenderTarget,
    UnorderedAccess,
    DepthRead,
    DepthWrite,
    ShaderResource,
    PixelShaderResource,
    CopyDest,
    CopySource,
    Present
};

} /* namespace RHI */
