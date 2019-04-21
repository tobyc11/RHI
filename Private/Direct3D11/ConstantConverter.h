#pragma once
#include "D3D11Platform.h"
#include "Format.h"
#include "PipelineStateDesc.h"
#include "RHIException.h"
#include "Resources.h"
#include "Sampler.h"

namespace RHI
{

DXGI_FORMAT Convert(EFormat format);
D3D11_TEXTURE_ADDRESS_MODE Convert(ESamplerAddressMode mode);
D3D11_FILL_MODE Convert(EPolygonMode mode);
D3D11_CULL_MODE Convert(ECullModeFlags cull);
D3D11_COMPARISON_FUNC Convert(ECompareOp op);
D3D11_STENCIL_OP Convert(EStencilOp op);
D3D11_BLEND Convert(EBlendMode mode);
D3D11_BLEND_OP Convert(EBlendOp op);
D3D11_PRIMITIVE_TOPOLOGY Convert(EPrimitiveTopology op);
D3D11_SHADER_RESOURCE_VIEW_DESC ConvertDescToSRV(const CImageViewDesc& desc);
D3D11_RENDER_TARGET_VIEW_DESC ConvertDescToRTV(const CImageViewDesc& desc);
D3D11_DEPTH_STENCIL_VIEW_DESC ConvertDescToDSV(const CImageViewDesc& desc);

inline bool IsDepthStencilFormat(EFormat format)
{
    switch (format)
    {
    case EFormat::D16_UNORM:
    case EFormat::X8_D24_UNORM_PACK32:
    case EFormat::D32_SFLOAT:
    case EFormat::D24_UNORM_S8_UINT:
    case EFormat::D32_SFLOAT_S8_UINT:
        return true;
    default:
        return false;
    }
}

inline DXGI_FORMAT DepthStencilFormatToTypeless(EFormat format)
{
    switch (format)
    {
    case EFormat::D16_UNORM:
        return DXGI_FORMAT_R16_TYPELESS;
    case EFormat::X8_D24_UNORM_PACK32:
        return DXGI_FORMAT_R24G8_TYPELESS;
    case EFormat::D32_SFLOAT:
        return DXGI_FORMAT_R32_TYPELESS;
    case EFormat::D24_UNORM_S8_UINT:
        return DXGI_FORMAT_R24G8_TYPELESS;
    case EFormat::D32_SFLOAT_S8_UINT:
        return DXGI_FORMAT_R32G8X24_TYPELESS;
    default:
        throw CRHIException("unsupported format");
    }
}

inline DXGI_FORMAT DepthStencilFormatToDepth(EFormat format)
{
    switch (format)
    {
    case EFormat::D16_UNORM:
        return DXGI_FORMAT_R16_FLOAT;
    case EFormat::X8_D24_UNORM_PACK32:
        return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    case EFormat::D32_SFLOAT:
        return DXGI_FORMAT_R32_FLOAT;
    case EFormat::D24_UNORM_S8_UINT:
        return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    case EFormat::D32_SFLOAT_S8_UINT:
        return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
    default:
        throw CRHIException("unsupported format");
    }
}

inline DXGI_FORMAT DepthStencilFormatToStencil(EFormat format)
{
    switch (format)
    {
    case EFormat::D24_UNORM_S8_UINT:
        return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
    case EFormat::D32_SFLOAT_S8_UINT:
        return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
    default:
        throw CRHIException("unsupported format");
    }
}

} /* namespace RHI */
