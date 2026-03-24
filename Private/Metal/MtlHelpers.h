#pragma once
#include "MtlCommon.h"
#include "Format.h"
#include "PipelineStateDesc.h"
#include "RenderPass.h"
#include "Resources.h"
#include "Sampler.h"
#include "ShaderModule.h"

namespace RHI
{

inline MTLPixelFormat MtlCast(EFormat format)
{
    switch (format)
    {
    case EFormat::UNDEFINED: return MTLPixelFormatInvalid;

    case EFormat::R8_UNORM: return MTLPixelFormatR8Unorm;
    case EFormat::R8_SNORM: return MTLPixelFormatR8Snorm;
    case EFormat::R8_UINT: return MTLPixelFormatR8Uint;
    case EFormat::R8_SINT: return MTLPixelFormatR8Sint;

    case EFormat::R8G8_UNORM: return MTLPixelFormatRG8Unorm;
    case EFormat::R8G8_SNORM: return MTLPixelFormatRG8Snorm;
    case EFormat::R8G8_UINT: return MTLPixelFormatRG8Uint;
    case EFormat::R8G8_SINT: return MTLPixelFormatRG8Sint;

    case EFormat::R8G8B8A8_UNORM: return MTLPixelFormatRGBA8Unorm;
    case EFormat::R8G8B8A8_SNORM: return MTLPixelFormatRGBA8Snorm;
    case EFormat::R8G8B8A8_UINT: return MTLPixelFormatRGBA8Uint;
    case EFormat::R8G8B8A8_SINT: return MTLPixelFormatRGBA8Sint;
    case EFormat::R8G8B8A8_SRGB: return MTLPixelFormatRGBA8Unorm_sRGB;

    case EFormat::B8G8R8A8_UNORM: return MTLPixelFormatBGRA8Unorm;
    case EFormat::B8G8R8A8_SRGB: return MTLPixelFormatBGRA8Unorm_sRGB;

    case EFormat::A8B8G8R8_UNORM_PACK32: return MTLPixelFormatRGBA8Unorm;
    case EFormat::A8B8G8R8_SNORM_PACK32: return MTLPixelFormatRGBA8Snorm;
    case EFormat::A8B8G8R8_UINT_PACK32: return MTLPixelFormatRGBA8Uint;
    case EFormat::A8B8G8R8_SINT_PACK32: return MTLPixelFormatRGBA8Sint;
    case EFormat::A8B8G8R8_SRGB_PACK32: return MTLPixelFormatRGBA8Unorm_sRGB;

    case EFormat::A2R10G10B10_UNORM_PACK32: return MTLPixelFormatBGR10A2Unorm;
    case EFormat::A2B10G10R10_UNORM_PACK32: return MTLPixelFormatRGB10A2Unorm;
    case EFormat::A2B10G10R10_UINT_PACK32: return MTLPixelFormatRGB10A2Uint;

    case EFormat::R16_UNORM: return MTLPixelFormatR16Unorm;
    case EFormat::R16_SNORM: return MTLPixelFormatR16Snorm;
    case EFormat::R16_UINT: return MTLPixelFormatR16Uint;
    case EFormat::R16_SINT: return MTLPixelFormatR16Sint;
    case EFormat::R16_SFLOAT: return MTLPixelFormatR16Float;

    case EFormat::R16G16_UNORM: return MTLPixelFormatRG16Unorm;
    case EFormat::R16G16_SNORM: return MTLPixelFormatRG16Snorm;
    case EFormat::R16G16_UINT: return MTLPixelFormatRG16Uint;
    case EFormat::R16G16_SINT: return MTLPixelFormatRG16Sint;
    case EFormat::R16G16_SFLOAT: return MTLPixelFormatRG16Float;

    case EFormat::R16G16B16A16_UNORM: return MTLPixelFormatRGBA16Unorm;
    case EFormat::R16G16B16A16_SNORM: return MTLPixelFormatRGBA16Snorm;
    case EFormat::R16G16B16A16_UINT: return MTLPixelFormatRGBA16Uint;
    case EFormat::R16G16B16A16_SINT: return MTLPixelFormatRGBA16Sint;
    case EFormat::R16G16B16A16_SFLOAT: return MTLPixelFormatRGBA16Float;

    case EFormat::R32_UINT: return MTLPixelFormatR32Uint;
    case EFormat::R32_SINT: return MTLPixelFormatR32Sint;
    case EFormat::R32_SFLOAT: return MTLPixelFormatR32Float;

    case EFormat::R32G32_UINT: return MTLPixelFormatRG32Uint;
    case EFormat::R32G32_SINT: return MTLPixelFormatRG32Sint;
    case EFormat::R32G32_SFLOAT: return MTLPixelFormatRG32Float;

    case EFormat::R32G32B32A32_UINT: return MTLPixelFormatRGBA32Uint;
    case EFormat::R32G32B32A32_SINT: return MTLPixelFormatRGBA32Sint;
    case EFormat::R32G32B32A32_SFLOAT: return MTLPixelFormatRGBA32Float;

    case EFormat::B10G11R11_UFLOAT_PACK32: return MTLPixelFormatRG11B10Float;
    case EFormat::E5B9G9R9_UFLOAT_PACK32: return MTLPixelFormatRGB9E5Float;

    case EFormat::D16_UNORM: return MTLPixelFormatDepth16Unorm;
    case EFormat::D32_SFLOAT: return MTLPixelFormatDepth32Float;
    case EFormat::S8_UINT: return MTLPixelFormatStencil8;
    case EFormat::D24_UNORM_S8_UINT: return MTLPixelFormatDepth24Unorm_Stencil8;
    case EFormat::D32_SFLOAT_S8_UINT: return MTLPixelFormatDepth32Float_Stencil8;

    case EFormat::BC1_RGB_UNORM_BLOCK: return MTLPixelFormatBC1_RGBA;
    case EFormat::BC1_RGB_SRGB_BLOCK: return MTLPixelFormatBC1_RGBA_sRGB;
    case EFormat::BC1_RGBA_UNORM_BLOCK: return MTLPixelFormatBC1_RGBA;
    case EFormat::BC1_RGBA_SRGB_BLOCK: return MTLPixelFormatBC1_RGBA_sRGB;
    case EFormat::BC2_UNORM_BLOCK: return MTLPixelFormatBC2_RGBA;
    case EFormat::BC2_SRGB_BLOCK: return MTLPixelFormatBC2_RGBA_sRGB;
    case EFormat::BC3_UNORM_BLOCK: return MTLPixelFormatBC3_RGBA;
    case EFormat::BC3_SRGB_BLOCK: return MTLPixelFormatBC3_RGBA_sRGB;
    case EFormat::BC4_UNORM_BLOCK: return MTLPixelFormatBC4_RUnorm;
    case EFormat::BC4_SNORM_BLOCK: return MTLPixelFormatBC4_RSnorm;
    case EFormat::BC5_UNORM_BLOCK: return MTLPixelFormatBC5_RGUnorm;
    case EFormat::BC5_SNORM_BLOCK: return MTLPixelFormatBC5_RGSnorm;
    case EFormat::BC6H_UFLOAT_BLOCK: return MTLPixelFormatBC6H_RGBUfloat;
    case EFormat::BC6H_SFLOAT_BLOCK: return MTLPixelFormatBC6H_RGBFloat;
    case EFormat::BC7_UNORM_BLOCK: return MTLPixelFormatBC7_RGBAUnorm;
    case EFormat::BC7_SRGB_BLOCK: return MTLPixelFormatBC7_RGBAUnorm_sRGB;

    case EFormat::ETC2_R8G8B8_UNORM_BLOCK: return MTLPixelFormatETC2_RGB8;
    case EFormat::ETC2_R8G8B8_SRGB_BLOCK: return MTLPixelFormatETC2_RGB8_sRGB;
    case EFormat::ETC2_R8G8B8A1_UNORM_BLOCK: return MTLPixelFormatETC2_RGB8A1;
    case EFormat::ETC2_R8G8B8A1_SRGB_BLOCK: return MTLPixelFormatETC2_RGB8A1_sRGB;
    case EFormat::ETC2_R8G8B8A8_UNORM_BLOCK: return MTLPixelFormatEAC_RGBA8;
    case EFormat::ETC2_R8G8B8A8_SRGB_BLOCK: return MTLPixelFormatEAC_RGBA8_sRGB;
    case EFormat::EAC_R11_UNORM_BLOCK: return MTLPixelFormatEAC_R11Unorm;
    case EFormat::EAC_R11_SNORM_BLOCK: return MTLPixelFormatEAC_R11Snorm;
    case EFormat::EAC_R11G11_UNORM_BLOCK: return MTLPixelFormatEAC_RG11Unorm;
    case EFormat::EAC_R11G11_SNORM_BLOCK: return MTLPixelFormatEAC_RG11Snorm;

    case EFormat::ASTC_4x4_UNORM_BLOCK: return MTLPixelFormatASTC_4x4_LDR;
    case EFormat::ASTC_4x4_SRGB_BLOCK: return MTLPixelFormatASTC_4x4_sRGB;
    case EFormat::ASTC_5x4_UNORM_BLOCK: return MTLPixelFormatASTC_5x4_LDR;
    case EFormat::ASTC_5x4_SRGB_BLOCK: return MTLPixelFormatASTC_5x4_sRGB;
    case EFormat::ASTC_5x5_UNORM_BLOCK: return MTLPixelFormatASTC_5x5_LDR;
    case EFormat::ASTC_5x5_SRGB_BLOCK: return MTLPixelFormatASTC_5x5_sRGB;
    case EFormat::ASTC_6x5_UNORM_BLOCK: return MTLPixelFormatASTC_6x5_LDR;
    case EFormat::ASTC_6x5_SRGB_BLOCK: return MTLPixelFormatASTC_6x5_sRGB;
    case EFormat::ASTC_6x6_UNORM_BLOCK: return MTLPixelFormatASTC_6x6_LDR;
    case EFormat::ASTC_6x6_SRGB_BLOCK: return MTLPixelFormatASTC_6x6_sRGB;
    case EFormat::ASTC_8x5_UNORM_BLOCK: return MTLPixelFormatASTC_8x5_LDR;
    case EFormat::ASTC_8x5_SRGB_BLOCK: return MTLPixelFormatASTC_8x5_sRGB;
    case EFormat::ASTC_8x6_UNORM_BLOCK: return MTLPixelFormatASTC_8x6_LDR;
    case EFormat::ASTC_8x6_SRGB_BLOCK: return MTLPixelFormatASTC_8x6_sRGB;
    case EFormat::ASTC_8x8_UNORM_BLOCK: return MTLPixelFormatASTC_8x8_LDR;
    case EFormat::ASTC_8x8_SRGB_BLOCK: return MTLPixelFormatASTC_8x8_sRGB;
    case EFormat::ASTC_10x5_UNORM_BLOCK: return MTLPixelFormatASTC_10x5_LDR;
    case EFormat::ASTC_10x5_SRGB_BLOCK: return MTLPixelFormatASTC_10x5_sRGB;
    case EFormat::ASTC_10x6_UNORM_BLOCK: return MTLPixelFormatASTC_10x6_LDR;
    case EFormat::ASTC_10x6_SRGB_BLOCK: return MTLPixelFormatASTC_10x6_sRGB;
    case EFormat::ASTC_10x8_UNORM_BLOCK: return MTLPixelFormatASTC_10x8_LDR;
    case EFormat::ASTC_10x8_SRGB_BLOCK: return MTLPixelFormatASTC_10x8_sRGB;
    case EFormat::ASTC_10x10_UNORM_BLOCK: return MTLPixelFormatASTC_10x10_LDR;
    case EFormat::ASTC_10x10_SRGB_BLOCK: return MTLPixelFormatASTC_10x10_sRGB;
    case EFormat::ASTC_12x10_UNORM_BLOCK: return MTLPixelFormatASTC_12x10_LDR;
    case EFormat::ASTC_12x10_SRGB_BLOCK: return MTLPixelFormatASTC_12x10_sRGB;
    case EFormat::ASTC_12x12_UNORM_BLOCK: return MTLPixelFormatASTC_12x12_LDR;
    case EFormat::ASTC_12x12_SRGB_BLOCK: return MTLPixelFormatASTC_12x12_sRGB;

    default: return MTLPixelFormatInvalid;
    }
}

inline MTLVertexFormat MtlVertexFormat(EFormat format)
{
    switch (format)
    {
    case EFormat::R8_UNORM: return MTLVertexFormatUCharNormalized;
    case EFormat::R8_UINT: return MTLVertexFormatUChar;
    case EFormat::R8_SINT: return MTLVertexFormatChar;

    case EFormat::R8G8_UNORM: return MTLVertexFormatUChar2Normalized;
    case EFormat::R8G8_UINT: return MTLVertexFormatUChar2;
    case EFormat::R8G8_SINT: return MTLVertexFormatChar2;

    case EFormat::R8G8B8A8_UNORM: return MTLVertexFormatUChar4Normalized;
    case EFormat::R8G8B8A8_UINT: return MTLVertexFormatUChar4;
    case EFormat::R8G8B8A8_SINT: return MTLVertexFormatChar4;

    case EFormat::R16_UINT: return MTLVertexFormatUShort;
    case EFormat::R16_SINT: return MTLVertexFormatShort;
    case EFormat::R16_UNORM: return MTLVertexFormatUShortNormalized;
    case EFormat::R16_SNORM: return MTLVertexFormatShortNormalized;
    case EFormat::R16_SFLOAT: return MTLVertexFormatHalf;

    case EFormat::R16G16_UINT: return MTLVertexFormatUShort2;
    case EFormat::R16G16_SINT: return MTLVertexFormatShort2;
    case EFormat::R16G16_UNORM: return MTLVertexFormatUShort2Normalized;
    case EFormat::R16G16_SNORM: return MTLVertexFormatShort2Normalized;
    case EFormat::R16G16_SFLOAT: return MTLVertexFormatHalf2;

    case EFormat::R16G16B16A16_UINT: return MTLVertexFormatUShort4;
    case EFormat::R16G16B16A16_SINT: return MTLVertexFormatShort4;
    case EFormat::R16G16B16A16_UNORM: return MTLVertexFormatUShort4Normalized;
    case EFormat::R16G16B16A16_SNORM: return MTLVertexFormatShort4Normalized;
    case EFormat::R16G16B16A16_SFLOAT: return MTLVertexFormatHalf4;

    case EFormat::R32_UINT: return MTLVertexFormatUInt;
    case EFormat::R32_SINT: return MTLVertexFormatInt;
    case EFormat::R32_SFLOAT: return MTLVertexFormatFloat;

    case EFormat::R32G32_UINT: return MTLVertexFormatUInt2;
    case EFormat::R32G32_SINT: return MTLVertexFormatInt2;
    case EFormat::R32G32_SFLOAT: return MTLVertexFormatFloat2;

    case EFormat::R32G32B32_UINT: return MTLVertexFormatUInt3;
    case EFormat::R32G32B32_SINT: return MTLVertexFormatInt3;
    case EFormat::R32G32B32_SFLOAT: return MTLVertexFormatFloat3;

    case EFormat::R32G32B32A32_UINT: return MTLVertexFormatUInt4;
    case EFormat::R32G32B32A32_SINT: return MTLVertexFormatInt4;
    case EFormat::R32G32B32A32_SFLOAT: return MTLVertexFormatFloat4;

    default: return MTLVertexFormatInvalid;
    }
}

inline MTLCompareFunction MtlCast(ECompareOp op)
{
    switch (op)
    {
    case ECompareOp::Never: return MTLCompareFunctionNever;
    case ECompareOp::Less: return MTLCompareFunctionLess;
    case ECompareOp::Equal: return MTLCompareFunctionEqual;
    case ECompareOp::LessEqual: return MTLCompareFunctionLessEqual;
    case ECompareOp::Greater: return MTLCompareFunctionGreater;
    case ECompareOp::NotEqual: return MTLCompareFunctionNotEqual;
    case ECompareOp::GreaterEqual: return MTLCompareFunctionGreaterEqual;
    case ECompareOp::Always: return MTLCompareFunctionAlways;
    default: return MTLCompareFunctionAlways;
    }
}

inline MTLStencilOperation MtlCast(EStencilOp op)
{
    switch (op)
    {
    case EStencilOp::Keep: return MTLStencilOperationKeep;
    case EStencilOp::Zero: return MTLStencilOperationZero;
    case EStencilOp::Replace: return MTLStencilOperationReplace;
    case EStencilOp::IncrementAndClamp: return MTLStencilOperationIncrementClamp;
    case EStencilOp::DecrementAndClamp: return MTLStencilOperationDecrementClamp;
    case EStencilOp::Invert: return MTLStencilOperationInvert;
    case EStencilOp::IncrementAndWrap: return MTLStencilOperationIncrementWrap;
    case EStencilOp::DecrementAndWrap: return MTLStencilOperationDecrementWrap;
    default: return MTLStencilOperationKeep;
    }
}

inline MTLBlendFactor MtlCast(EBlendMode mode)
{
    switch (mode)
    {
    case EBlendMode::Zero: return MTLBlendFactorZero;
    case EBlendMode::One: return MTLBlendFactorOne;
    case EBlendMode::SrcColor: return MTLBlendFactorSourceColor;
    case EBlendMode::OneMinusSrcColor: return MTLBlendFactorOneMinusSourceColor;
    case EBlendMode::SrcAlpha: return MTLBlendFactorSourceAlpha;
    case EBlendMode::OneMinusSrcAlpha: return MTLBlendFactorOneMinusSourceAlpha;
    case EBlendMode::DstAlpha: return MTLBlendFactorDestinationAlpha;
    case EBlendMode::OneMinusDstAlpha: return MTLBlendFactorOneMinusDestinationAlpha;
    case EBlendMode::DstColor: return MTLBlendFactorDestinationColor;
    case EBlendMode::OneMinusDstColor: return MTLBlendFactorOneMinusDestinationColor;
    case EBlendMode::SrcAlphaSaturate: return MTLBlendFactorSourceAlphaSaturated;
    default: return MTLBlendFactorOne;
    }
}

inline MTLBlendOperation MtlCast(EBlendOp op)
{
    switch (op)
    {
    case EBlendOp::Add: return MTLBlendOperationAdd;
    case EBlendOp::Subtract: return MTLBlendOperationSubtract;
    case EBlendOp::ReverseSubtract: return MTLBlendOperationReverseSubtract;
    case EBlendOp::Min: return MTLBlendOperationMin;
    case EBlendOp::Max: return MTLBlendOperationMax;
    default: return MTLBlendOperationAdd;
    }
}

inline MTLColorWriteMask MtlCast(EColorComponentFlags flags)
{
    MTLColorWriteMask mask = MTLColorWriteMaskNone;
    if (static_cast<uint8_t>(flags) & static_cast<uint8_t>(EColorComponentFlags::Red))
        mask |= MTLColorWriteMaskRed;
    if (static_cast<uint8_t>(flags) & static_cast<uint8_t>(EColorComponentFlags::Green))
        mask |= MTLColorWriteMaskGreen;
    if (static_cast<uint8_t>(flags) & static_cast<uint8_t>(EColorComponentFlags::Blue))
        mask |= MTLColorWriteMaskBlue;
    if (static_cast<uint8_t>(flags) & static_cast<uint8_t>(EColorComponentFlags::Alpha))
        mask |= MTLColorWriteMaskAlpha;
    return mask;
}

inline MTLPrimitiveType MtlCast(EPrimitiveTopology topology)
{
    switch (topology)
    {
    case EPrimitiveTopology::PointList: return MTLPrimitiveTypePoint;
    case EPrimitiveTopology::LineList: return MTLPrimitiveTypeLine;
    case EPrimitiveTopology::LineStrip: return MTLPrimitiveTypeLineStrip;
    case EPrimitiveTopology::TriangleList: return MTLPrimitiveTypeTriangle;
    case EPrimitiveTopology::TriangleStrip: return MTLPrimitiveTypeTriangleStrip;
    default: return MTLPrimitiveTypeTriangle;
    }
}

inline MTLPrimitiveTopologyClass MtlTopologyClass(EPrimitiveTopology topology)
{
    switch (topology)
    {
    case EPrimitiveTopology::PointList: return MTLPrimitiveTopologyClassPoint;
    case EPrimitiveTopology::LineList:
    case EPrimitiveTopology::LineStrip: return MTLPrimitiveTopologyClassLine;
    case EPrimitiveTopology::TriangleList:
    case EPrimitiveTopology::TriangleStrip:
    case EPrimitiveTopology::TriangleFan: return MTLPrimitiveTopologyClassTriangle;
    default: return MTLPrimitiveTopologyClassTriangle;
    }
}

inline MTLCullMode MtlCast(ECullModeFlags cull)
{
    auto v = static_cast<uint32_t>(cull);
    if (v == 0)
        return MTLCullModeNone;
    if (v & static_cast<uint32_t>(ECullModeFlags::Front))
        return MTLCullModeFront;
    if (v & static_cast<uint32_t>(ECullModeFlags::Back))
        return MTLCullModeBack;
    return MTLCullModeNone;
}

inline MTLTriangleFillMode MtlCast(EPolygonMode mode)
{
    switch (mode)
    {
    case EPolygonMode::Fill: return MTLTriangleFillModeFill;
    case EPolygonMode::Wireframe: return MTLTriangleFillModeLines;
    default: return MTLTriangleFillModeFill;
    }
}

inline MTLWinding MtlWinding(bool frontFaceCCW)
{
    return frontFaceCCW ? MTLWindingCounterClockwise : MTLWindingClockwise;
}

inline MTLSamplerMinMagFilter MtlCast(EFilter filter)
{
    switch (filter)
    {
    case EFilter::Nearest: return MTLSamplerMinMagFilterNearest;
    case EFilter::Linear: return MTLSamplerMinMagFilterLinear;
    default: return MTLSamplerMinMagFilterLinear;
    }
}

inline MTLSamplerMipFilter MtlCast(ESamplerMipmapMode mode)
{
    switch (mode)
    {
    case ESamplerMipmapMode::Nearest: return MTLSamplerMipFilterNearest;
    case ESamplerMipmapMode::Linear: return MTLSamplerMipFilterLinear;
    default: return MTLSamplerMipFilterLinear;
    }
}

inline MTLSamplerAddressMode MtlCast(ESamplerAddressMode mode)
{
    switch (mode)
    {
    case ESamplerAddressMode::Wrap: return MTLSamplerAddressModeRepeat;
    case ESamplerAddressMode::Mirror: return MTLSamplerAddressModeMirrorRepeat;
    case ESamplerAddressMode::Clamp: return MTLSamplerAddressModeClampToEdge;
    case ESamplerAddressMode::Border: return MTLSamplerAddressModeClampToBorderColor;
    case ESamplerAddressMode::MirrorOnce: return MTLSamplerAddressModeMirrorClampToEdge;
    default: return MTLSamplerAddressModeClampToEdge;
    }
}

inline MTLLoadAction MtlCast(EAttachmentLoadOp op)
{
    switch (op)
    {
    case EAttachmentLoadOp::Load: return MTLLoadActionLoad;
    case EAttachmentLoadOp::Clear: return MTLLoadActionClear;
    case EAttachmentLoadOp::DontCare: return MTLLoadActionDontCare;
    default: return MTLLoadActionDontCare;
    }
}

inline MTLStoreAction MtlCast(EAttachmentStoreOp op)
{
    switch (op)
    {
    case EAttachmentStoreOp::Store: return MTLStoreActionStore;
    case EAttachmentStoreOp::DontCare: return MTLStoreActionDontCare;
    default: return MTLStoreActionDontCare;
    }
}

inline MTLIndexType MtlIndexType(EFormat format)
{
    switch (format)
    {
    case EFormat::R16_UINT: return MTLIndexTypeUInt16;
    case EFormat::R32_UINT: return MTLIndexTypeUInt32;
    default: return MTLIndexTypeUInt32;
    }
}

inline MTLTextureType MtlTextureType(EImageViewType type)
{
    switch (type)
    {
    case EImageViewType::View1D: return MTLTextureType1D;
    case EImageViewType::View2D: return MTLTextureType2D;
    case EImageViewType::View3D: return MTLTextureType3D;
    case EImageViewType::Cube: return MTLTextureTypeCube;
    case EImageViewType::View1DArray: return MTLTextureType1DArray;
    case EImageViewType::View2DArray: return MTLTextureType2DArray;
    case EImageViewType::CubeArray: return MTLTextureTypeCubeArray;
    default: return MTLTextureType2D;
    }
}

} /* namespace RHI */
