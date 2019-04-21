#include "ConstantConverter.h"
#include "RHIException.h"

namespace RHI
{

static DXGI_FORMAT FormatMappingTable[] = {
    DXGI_FORMAT_UNKNOWN, /* UNDEFINED */
    DXGI_FORMAT_UNKNOWN, /* R4G4_UNORM_PACK8 */
    DXGI_FORMAT_UNKNOWN, /* R4G4B4A4_UNORM_PACK16 */
    DXGI_FORMAT_UNKNOWN, /* B4G4R4A4_UNORM_PACK16 */
    DXGI_FORMAT_UNKNOWN, /* R5G6B5_UNORM_PACK16 */
    DXGI_FORMAT_UNKNOWN, /* B5G6R5_UNORM_PACK16 */
    DXGI_FORMAT_UNKNOWN, /* R5G5B5A1_UNORM_PACK16 */
    DXGI_FORMAT_UNKNOWN, /* B5G5R5A1_UNORM_PACK16 */
    DXGI_FORMAT_UNKNOWN, /* A1R5G5B5_UNORM_PACK16 */
    DXGI_FORMAT_R8_UNORM, /* R8_UNORM */
    DXGI_FORMAT_R8_SNORM, /* R8_SNORM */
    DXGI_FORMAT_UNKNOWN, /* R8_USCALED */
    DXGI_FORMAT_UNKNOWN, /* R8_SSCALED */
    DXGI_FORMAT_R8_UINT, /* R8_UINT */
    DXGI_FORMAT_R8_SINT, /* R8_SINT */
    DXGI_FORMAT_UNKNOWN, /* R8_SRGB */
    DXGI_FORMAT_R8G8_UNORM, /* R8G8_UNORM */
    DXGI_FORMAT_R8G8_SNORM, /* R8G8_SNORM */
    DXGI_FORMAT_UNKNOWN, /* R8G8_USCALED */
    DXGI_FORMAT_UNKNOWN, /* R8G8_SSCALED */
    DXGI_FORMAT_R8G8_UINT, /* R8G8_UINT */
    DXGI_FORMAT_R8G8_SINT, /* R8G8_SINT */
    DXGI_FORMAT_UNKNOWN, /* R8G8_SRGB */
    DXGI_FORMAT_UNKNOWN, /* R8G8B8_UNORM */
    DXGI_FORMAT_UNKNOWN, /* R8G8B8_SNORM */
    DXGI_FORMAT_UNKNOWN, /* R8G8B8_USCALED */
    DXGI_FORMAT_UNKNOWN, /* R8G8B8_SSCALED */
    DXGI_FORMAT_UNKNOWN, /* R8G8B8_UINT */
    DXGI_FORMAT_UNKNOWN, /* R8G8B8_SINT */
    DXGI_FORMAT_UNKNOWN, /* R8G8B8_SRGB */
    DXGI_FORMAT_UNKNOWN, /* B8G8R8_UNORM */
    DXGI_FORMAT_UNKNOWN, /* B8G8R8_SNORM */
    DXGI_FORMAT_UNKNOWN, /* B8G8R8_USCALED */
    DXGI_FORMAT_UNKNOWN, /* B8G8R8_SSCALED */
    DXGI_FORMAT_UNKNOWN, /* B8G8R8_UINT */
    DXGI_FORMAT_UNKNOWN, /* B8G8R8_SINT */
    DXGI_FORMAT_UNKNOWN, /* B8G8R8_SRGB */
    DXGI_FORMAT_R8G8B8A8_UNORM, /* R8G8B8A8_UNORM */
    DXGI_FORMAT_R8G8B8A8_SNORM, /* R8G8B8A8_SNORM */
    DXGI_FORMAT_R8G8B8A8_TYPELESS, /* R8G8B8A8_USCALED */
    DXGI_FORMAT_R8G8B8A8_TYPELESS, /* R8G8B8A8_SSCALED */
    DXGI_FORMAT_R8G8B8A8_UINT, /* R8G8B8A8_UINT */
    DXGI_FORMAT_R8G8B8A8_SINT, /* R8G8B8A8_SINT */
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, /* R8G8B8A8_SRGB */
    DXGI_FORMAT_B8G8R8A8_UNORM, /* B8G8R8A8_UNORM */
    DXGI_FORMAT_B8G8R8A8_TYPELESS, /* B8G8R8A8_SNORM */
    DXGI_FORMAT_B8G8R8A8_TYPELESS, /* B8G8R8A8_USCALED */
    DXGI_FORMAT_B8G8R8A8_TYPELESS, /* B8G8R8A8_SSCALED */
    DXGI_FORMAT_B8G8R8A8_TYPELESS, /* B8G8R8A8_UINT */
    DXGI_FORMAT_B8G8R8A8_TYPELESS, /* B8G8R8A8_SINT */
    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, /* B8G8R8A8_SRGB */
    DXGI_FORMAT_UNKNOWN, /* A8B8G8R8_UNORM_PACK32 */
    DXGI_FORMAT_UNKNOWN, /* A8B8G8R8_SNORM_PACK32 */
    DXGI_FORMAT_UNKNOWN, /* A8B8G8R8_USCALED_PACK32 */
    DXGI_FORMAT_UNKNOWN, /* A8B8G8R8_SSCALED_PACK32 */
    DXGI_FORMAT_UNKNOWN, /* A8B8G8R8_UINT_PACK32 */
    DXGI_FORMAT_UNKNOWN, /* A8B8G8R8_SINT_PACK32 */
    DXGI_FORMAT_UNKNOWN, /* A8B8G8R8_SRGB_PACK32 */
    DXGI_FORMAT_R10G10B10A2_UNORM, /* A2R10G10B10_UNORM_PACK32 */
    DXGI_FORMAT_UNKNOWN, /* A2R10G10B10_SNORM_PACK32 */
    DXGI_FORMAT_UNKNOWN, /* A2R10G10B10_USCALED_PACK32 */
    DXGI_FORMAT_UNKNOWN, /* A2R10G10B10_SSCALED_PACK32 */
    DXGI_FORMAT_R10G10B10A2_UINT, /* A2R10G10B10_UINT_PACK32 */
    DXGI_FORMAT_UNKNOWN, /* A2R10G10B10_SINT_PACK32 */
    DXGI_FORMAT_UNKNOWN, /* A2B10G10R10_UNORM_PACK32 */
    DXGI_FORMAT_UNKNOWN, /* A2B10G10R10_SNORM_PACK32 */
    DXGI_FORMAT_UNKNOWN, /* A2B10G10R10_USCALED_PACK32 */
    DXGI_FORMAT_UNKNOWN, /* A2B10G10R10_SSCALED_PACK32 */
    DXGI_FORMAT_UNKNOWN, /* A2B10G10R10_UINT_PACK32 */
    DXGI_FORMAT_UNKNOWN, /* A2B10G10R10_SINT_PACK32 */
    DXGI_FORMAT_R16_UNORM, /* R16_UNORM */
    DXGI_FORMAT_R16_SNORM, /* R16_SNORM */
    DXGI_FORMAT_R16_TYPELESS, /* R16_USCALED */
    DXGI_FORMAT_R16_TYPELESS, /* R16_SSCALED */
    DXGI_FORMAT_R16_UINT, /* R16_UINT */
    DXGI_FORMAT_R16_SINT, /* R16_SINT */
    DXGI_FORMAT_R16_FLOAT, /* R16_SFLOAT */
    DXGI_FORMAT_R16G16_UNORM, /* R16G16_UNORM */
    DXGI_FORMAT_R16G16_SNORM, /* R16G16_SNORM */
    DXGI_FORMAT_R16G16_TYPELESS, /* R16G16_USCALED */
    DXGI_FORMAT_R16G16_TYPELESS, /* R16G16_SSCALED */
    DXGI_FORMAT_R16G16_UINT, /* R16G16_UINT */
    DXGI_FORMAT_R16G16_SINT, /* R16G16_SINT */
    DXGI_FORMAT_R16G16_FLOAT, /* R16G16_SFLOAT */
    DXGI_FORMAT_UNKNOWN, /* R16G16B16_UNORM */
    DXGI_FORMAT_UNKNOWN, /* R16G16B16_SNORM */
    DXGI_FORMAT_UNKNOWN, /* R16G16B16_USCALED */
    DXGI_FORMAT_UNKNOWN, /* R16G16B16_SSCALED */
    DXGI_FORMAT_UNKNOWN, /* R16G16B16_UINT */
    DXGI_FORMAT_UNKNOWN, /* R16G16B16_SINT */
    DXGI_FORMAT_UNKNOWN, /* R16G16B16_SFLOAT */
    DXGI_FORMAT_R16G16B16A16_UNORM, /* R16G16B16A16_UNORM */
    DXGI_FORMAT_R16G16B16A16_SNORM, /* R16G16B16A16_SNORM */
    DXGI_FORMAT_R16G16B16A16_TYPELESS, /* R16G16B16A16_USCALED */
    DXGI_FORMAT_R16G16B16A16_TYPELESS, /* R16G16B16A16_SSCALED */
    DXGI_FORMAT_R16G16B16A16_UINT, /* R16G16B16A16_UINT */
    DXGI_FORMAT_R16G16B16A16_SINT, /* R16G16B16A16_SINT */
    DXGI_FORMAT_R16G16B16A16_FLOAT, /* R16G16B16A16_SFLOAT */
    DXGI_FORMAT_R32_UINT, /* R32_UINT */
    DXGI_FORMAT_R32_SINT, /* R32_SINT */
    DXGI_FORMAT_R32_FLOAT, /* R32_SFLOAT */
    DXGI_FORMAT_R32G32_UINT, /* R32G32_UINT */
    DXGI_FORMAT_R32G32_SINT, /* R32G32_SINT */
    DXGI_FORMAT_R32G32_FLOAT, /* R32G32_SFLOAT */
    DXGI_FORMAT_R32G32B32_UINT, /* R32G32B32_UINT */
    DXGI_FORMAT_R32G32B32_SINT, /* R32G32B32_SINT */
    DXGI_FORMAT_R32G32B32_FLOAT, /* R32G32B32_SFLOAT */
    DXGI_FORMAT_R32G32B32A32_UINT, /* R32G32B32A32_UINT */
    DXGI_FORMAT_R32G32B32A32_SINT, /* R32G32B32A32_SINT */
    DXGI_FORMAT_R32G32B32A32_FLOAT, /* R32G32B32A32_SFLOAT */
    DXGI_FORMAT_UNKNOWN, /* R64_UINT */
    DXGI_FORMAT_UNKNOWN, /* R64_SINT */
    DXGI_FORMAT_UNKNOWN, /* R64_SFLOAT */
    DXGI_FORMAT_UNKNOWN, /* R64G64_UINT */
    DXGI_FORMAT_UNKNOWN, /* R64G64_SINT */
    DXGI_FORMAT_UNKNOWN, /* R64G64_SFLOAT */
    DXGI_FORMAT_UNKNOWN, /* R64G64B64_UINT */
    DXGI_FORMAT_UNKNOWN, /* R64G64B64_SINT */
    DXGI_FORMAT_UNKNOWN, /* R64G64B64_SFLOAT */
    DXGI_FORMAT_UNKNOWN, /* R64G64B64A64_UINT */
    DXGI_FORMAT_UNKNOWN, /* R64G64B64A64_SINT */
    DXGI_FORMAT_UNKNOWN, /* R64G64B64A64_SFLOAT */
    DXGI_FORMAT_R11G11B10_FLOAT, /* B10G11R11_UFLOAT_PACK32 */
    DXGI_FORMAT_UNKNOWN, /* E5B9G9R9_UFLOAT_PACK32 */
    DXGI_FORMAT_D16_UNORM, /* D16_UNORM */
    DXGI_FORMAT_D24_UNORM_S8_UINT, /* X8_D24_UNORM_PACK32 */
    DXGI_FORMAT_D32_FLOAT, /* D32_SFLOAT */
    DXGI_FORMAT_UNKNOWN, /* S8_UINT */
    DXGI_FORMAT_UNKNOWN, /* D16_UNORM_S8_UINT */
    DXGI_FORMAT_D24_UNORM_S8_UINT, /* D24_UNORM_S8_UINT */
    DXGI_FORMAT_D32_FLOAT_S8X24_UINT, /* D32_SFLOAT_S8_UINT */
    DXGI_FORMAT_UNKNOWN, /* BC1_RGB_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* BC1_RGB_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* BC1_RGBA_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* BC1_RGBA_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* BC2_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* BC2_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* BC3_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* BC3_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* BC4_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* BC4_SNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* BC5_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* BC5_SNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* BC6H_UFLOAT_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* BC6H_SFLOAT_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* BC7_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* BC7_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ETC2_R8G8B8_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ETC2_R8G8B8_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ETC2_R8G8B8A1_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ETC2_R8G8B8A1_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ETC2_R8G8B8A8_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ETC2_R8G8B8A8_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* EAC_R11_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* EAC_R11_SNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* EAC_R11G11_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* EAC_R11G11_SNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_4x4_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_4x4_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_5x4_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_5x4_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_5x5_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_5x5_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_6x5_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_6x5_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_6x6_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_6x6_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_8x5_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_8x5_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_8x6_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_8x6_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_8x8_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_8x8_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_10x5_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_10x5_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_10x6_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_10x6_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_10x8_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_10x8_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_10x10_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_10x10_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_12x10_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_12x10_SRGB_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_12x12_UNORM_BLOCK */
    DXGI_FORMAT_UNKNOWN, /* ASTC_12x12_SRGB_BLOCK */
};

DXGI_FORMAT Convert(EFormat format)
{
    DXGI_FORMAT result = FormatMappingTable[static_cast<size_t>(format)];
    if (result == DXGI_FORMAT_UNKNOWN && format != EFormat::UNDEFINED)
        throw CRHIException("You are using a format unsupported by the D3D11 backend");
}

D3D11_TEXTURE_ADDRESS_MODE Convert(ESamplerAddressMode mode)
{
    static_assert(D3D11_TEXTURE_ADDRESS_WRAP == static_cast<uint32_t>(ESamplerAddressMode::Wrap),
                  "def mismatch");
    static_assert(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE
                      == static_cast<uint32_t>(ESamplerAddressMode::MirrorOnce),
                  "def mismatch");
    return static_cast<D3D11_TEXTURE_ADDRESS_MODE>(mode);
}

D3D11_FILL_MODE Convert(EPolygonMode mode)
{
    switch (mode)
    {
    case EPolygonMode::Fill:
        return D3D11_FILL_SOLID;
    case EPolygonMode::Wireframe:
        return D3D11_FILL_WIREFRAME;
    }
    return D3D11_FILL_SOLID;
}

D3D11_CULL_MODE Convert(ECullModeFlags cull)
{
    D3D11_CULL_MODE result = D3D11_CULL_NONE;
    if (Any(cull, ECullModeFlags::Front))
        result = D3D11_CULL_FRONT;
    if (Any(cull, ECullModeFlags::Back))
        result = D3D11_CULL_BACK;
    return result;
}

D3D11_COMPARISON_FUNC Convert(ECompareOp op)
{
    switch (op)
    {
    case ECompareOp::Never:
        return D3D11_COMPARISON_NEVER;
    case ECompareOp::Less:
        return D3D11_COMPARISON_LESS;
    case ECompareOp::Equal:
        return D3D11_COMPARISON_EQUAL;
    case ECompareOp::LessEqual:
        return D3D11_COMPARISON_LESS_EQUAL;
    case ECompareOp::Greater:
        return D3D11_COMPARISON_GREATER;
    case ECompareOp::NotEqual:
        return D3D11_COMPARISON_NOT_EQUAL;
    case ECompareOp::GreaterEqual:
        return D3D11_COMPARISON_GREATER_EQUAL;
    case ECompareOp::Always:
        return D3D11_COMPARISON_ALWAYS;
    default:
        return D3D11_COMPARISON_ALWAYS;
    }
}

D3D11_STENCIL_OP Convert(EStencilOp op)
{
    switch (op)
    {
    case EStencilOp::Keep:
        return D3D11_STENCIL_OP_KEEP;
    case EStencilOp::Zero:
        return D3D11_STENCIL_OP_ZERO;
    case EStencilOp::Replace:
        return D3D11_STENCIL_OP_REPLACE;
    case EStencilOp::IncrementAndClamp:
        return D3D11_STENCIL_OP_INCR_SAT;
    case EStencilOp::DecrementAndClamp:
        return D3D11_STENCIL_OP_DECR_SAT;
    case EStencilOp::Invert:
        return D3D11_STENCIL_OP_INVERT;
    case EStencilOp::IncrementAndWrap:
        return D3D11_STENCIL_OP_INCR;
    case EStencilOp::DecrementAndWrap:
        return D3D11_STENCIL_OP_DECR;
    default:
        return D3D11_STENCIL_OP_KEEP;
    }
}

D3D11_BLEND Convert(EBlendMode mode)
{
    switch (mode)
    {
    case EBlendMode::Zero:
        return D3D11_BLEND_ZERO;
    case EBlendMode::One:
        return D3D11_BLEND_ONE;
    case EBlendMode::SrcColor:
        return D3D11_BLEND_SRC_COLOR;
    case EBlendMode::OneMinusSrcColor:
        return D3D11_BLEND_INV_SRC_COLOR;
    case EBlendMode::SrcAlpha:
        return D3D11_BLEND_SRC_ALPHA;
    case EBlendMode::OneMinusSrcAlpha:
        return D3D11_BLEND_INV_SRC_ALPHA;
    case EBlendMode::DstAlpha:
        return D3D11_BLEND_DEST_ALPHA;
    case EBlendMode::OneMinusDstAlpha:
        return D3D11_BLEND_INV_DEST_ALPHA;
    case EBlendMode::DstColor:
        return D3D11_BLEND_DEST_COLOR;
    case EBlendMode::OneMinusDstColor:
        return D3D11_BLEND_INV_DEST_COLOR;
    case EBlendMode::SrcAlphaSaturate:
        return D3D11_BLEND_SRC_ALPHA_SAT;
    default:
        return D3D11_BLEND_ZERO;
    }
}

D3D11_BLEND_OP Convert(EBlendOp op)
{
    switch (op)
    {
    case EBlendOp::Add:
        return D3D11_BLEND_OP_ADD;
    case EBlendOp::Subtract:
        return D3D11_BLEND_OP_SUBTRACT;
    case EBlendOp::ReverseSubtract:
        return D3D11_BLEND_OP_REV_SUBTRACT;
    case EBlendOp::Min:
        return D3D11_BLEND_OP_MIN;
    case EBlendOp::Max:
        return D3D11_BLEND_OP_MAX;
    default:
        return D3D11_BLEND_OP_ADD;
    }
}

D3D11_PRIMITIVE_TOPOLOGY Convert(EPrimitiveTopology op)
{
    switch (op)
    {
    case RHI::EPrimitiveTopology::PointList:
        return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    case RHI::EPrimitiveTopology::LineList:
        return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
    case RHI::EPrimitiveTopology::LineStrip:
        return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
    case RHI::EPrimitiveTopology::TriangleList:
        return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case RHI::EPrimitiveTopology::TriangleStrip:
        return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    case RHI::EPrimitiveTopology::TriangleFan:
    default:
        return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    }
}

D3D11_SHADER_RESOURCE_VIEW_DESC ConvertDescToSRV(const CImageViewDesc& desc)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC d;
    d.Format = Convert(desc.Format);
    if (IsDepthStencilFormat(desc.Format))
    {
        if (Any(desc.DepthStencilAspect, EDepthStencilAspectFlags::Depth))
            d.Format = DepthStencilFormatToDepth(desc.Format);
        else
            d.Format = DepthStencilFormatToStencil(desc.Format);
    }
    switch (desc.Type)
    {
    case EImageViewType::View1D:
        d.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
        d.Texture1D.MostDetailedMip = desc.Range.BaseMipLevel;
        d.Texture1D.MipLevels = desc.Range.LevelCount;
        break;
    case EImageViewType::View1DArray:
        d.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
        d.Texture1DArray.MostDetailedMip = desc.Range.BaseMipLevel;
        d.Texture1DArray.MipLevels = desc.Range.LevelCount;
        d.Texture1DArray.FirstArraySlice = desc.Range.BaseArrayLayer;
        d.Texture1DArray.ArraySize = desc.Range.LayerCount;
        break;
    case EImageViewType::View2D:
        d.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        d.Texture2D.MostDetailedMip = desc.Range.BaseMipLevel;
        d.Texture2D.MipLevels = desc.Range.LevelCount;
        break;
    case EImageViewType::View2DArray:
        d.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        d.Texture2DArray.MostDetailedMip = desc.Range.BaseMipLevel;
        d.Texture2DArray.MipLevels = desc.Range.LevelCount;
        d.Texture2DArray.FirstArraySlice = desc.Range.BaseArrayLayer;
        d.Texture2DArray.ArraySize = desc.Range.LayerCount;
        break;
    case EImageViewType::View3D:
        d.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
        d.Texture3D.MostDetailedMip = desc.Range.BaseMipLevel;
        d.Texture3D.MipLevels = desc.Range.LevelCount;
        break;
    case EImageViewType::Cube:
        d.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        d.TextureCube.MostDetailedMip = desc.Range.BaseMipLevel;
        d.TextureCube.MipLevels = desc.Range.LevelCount;
        break;
    case EImageViewType::CubeArray:
        d.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
        d.TextureCubeArray.MostDetailedMip = desc.Range.BaseMipLevel;
        d.TextureCubeArray.MipLevels = desc.Range.LevelCount;
        d.TextureCubeArray.First2DArrayFace = desc.Range.BaseArrayLayer;
        d.TextureCubeArray.NumCubes = desc.Range.LayerCount / 6; // TODO: confirm this
        break;
    default:
        throw CRHIRuntimeError("EImageViewType unrecognized");
    }
    return d;
}

D3D11_RENDER_TARGET_VIEW_DESC ConvertDescToRTV(const CImageViewDesc& desc)
{
    D3D11_RENDER_TARGET_VIEW_DESC d;
    d.Format = Convert(desc.Format);
    switch (desc.Type)
    {
    case EImageViewType::View1D:
        d.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
        d.Texture1D.MipSlice = desc.Range.BaseMipLevel;
        break;
    case EImageViewType::View1DArray:
        d.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
        d.Texture1DArray.MipSlice = desc.Range.BaseMipLevel;
        d.Texture1DArray.FirstArraySlice = desc.Range.BaseArrayLayer;
        d.Texture1DArray.ArraySize = desc.Range.LayerCount;
        break;
    case EImageViewType::View2D:
        d.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        d.Texture2D.MipSlice = desc.Range.BaseMipLevel;
        break;
    case EImageViewType::View2DArray:
        d.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        d.Texture2DArray.MipSlice = desc.Range.BaseMipLevel;
        d.Texture2DArray.FirstArraySlice = desc.Range.BaseArrayLayer;
        d.Texture2DArray.ArraySize = desc.Range.LayerCount;
        break;
    case EImageViewType::View3D:
        d.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
        d.Texture3D.MipSlice = desc.Range.BaseMipLevel;
        d.Texture3D.FirstWSlice = desc.Range.BaseArrayLayer;
        d.Texture3D.WSize = desc.Range.LayerCount;
        break;
    default:
        throw CRHIRuntimeError("EImageViewType unrecognized or unsuitable for render target view");
    }
    return d;
}

D3D11_DEPTH_STENCIL_VIEW_DESC ConvertDescToDSV(const CImageViewDesc& desc)
{
    D3D11_DEPTH_STENCIL_VIEW_DESC d;
    d.Format = Convert(desc.Format);
    d.Flags = 0;
    switch (desc.Type)
    {
    case EImageViewType::View1D:
        d.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1D;
        d.Texture1D.MipSlice = desc.Range.BaseMipLevel;
        break;
    case EImageViewType::View1DArray:
        d.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
        d.Texture1DArray.MipSlice = desc.Range.BaseMipLevel;
        d.Texture1DArray.FirstArraySlice = desc.Range.BaseArrayLayer;
        d.Texture1DArray.ArraySize = desc.Range.LayerCount;
        break;
    case EImageViewType::View2D:
        d.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        d.Texture2D.MipSlice = desc.Range.BaseMipLevel;
        break;
    case EImageViewType::View2DArray:
        d.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        d.Texture2DArray.MipSlice = desc.Range.BaseMipLevel;
        d.Texture2DArray.FirstArraySlice = desc.Range.BaseArrayLayer;
        d.Texture2DArray.ArraySize = desc.Range.LayerCount;
        break;
    default:
        throw CRHIRuntimeError("EImageViewType unrecognized or unsuitable for depth stencil view");
    }
    return d;
}

} /* namespace RHI */
