#pragma once
#include "PipelineStateDesc.h"
#include "Sampler.h"
#include "VkCommon.h"
#include <cassert>

namespace RHI
{

inline VkFilter VkCast(EFilter r)
{
    if (r == EFilter::Nearest)
        return VK_FILTER_NEAREST;
    return VK_FILTER_LINEAR;
}

inline VkSamplerMipmapMode VkCast(ESamplerMipmapMode r)
{
    if (r == ESamplerMipmapMode::Nearest)
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    return VK_SAMPLER_MIPMAP_MODE_LINEAR;
}

inline VkSamplerAddressMode VkCast(ESamplerAddressMode r)
{
    switch (r)
    {
    case ESamplerAddressMode::Wrap:
    default:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case ESamplerAddressMode::Mirror:
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case ESamplerAddressMode::Clamp:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case ESamplerAddressMode::Border:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    case ESamplerAddressMode::MirrorOnce:
        return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    }
}

inline VkCompareOp VkCast(ECompareOp r)
{
    switch (r)
    {
    case ECompareOp::Never:
    default:
        return VK_COMPARE_OP_NEVER;
    case ECompareOp::Less:
        return VK_COMPARE_OP_LESS;
    case ECompareOp::Equal:
        return VK_COMPARE_OP_EQUAL;
    case ECompareOp::LessEqual:
        return VK_COMPARE_OP_LESS_OR_EQUAL;
    case ECompareOp::Greater:
        return VK_COMPARE_OP_GREATER;
    case ECompareOp::NotEqual:
        return VK_COMPARE_OP_NOT_EQUAL;
    case ECompareOp::GreaterEqual:
        return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case ECompareOp::Always:
        return VK_COMPARE_OP_ALWAYS;
    }
}

inline VkStencilOp VkCast(EStencilOp r)
{
    switch (r)
    {
    case EStencilOp::Keep:
    default:
        return VK_STENCIL_OP_KEEP;
    case EStencilOp::Zero:
        return VK_STENCIL_OP_ZERO;
    case EStencilOp::Replace:
        return VK_STENCIL_OP_REPLACE;
    case EStencilOp::IncrementAndClamp:
        return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case EStencilOp::DecrementAndClamp:
        return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case EStencilOp::Invert:
        return VK_STENCIL_OP_INVERT;
    case EStencilOp::IncrementAndWrap:
        return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    case EStencilOp::DecrementAndWrap:
        return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    }
}

inline VkImageAspectFlags GetImageAspectFlags(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D32_SFLOAT:
        return VK_IMAGE_ASPECT_DEPTH_BIT;

    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    default:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

inline uint32_t GetUncompressedImageFormatSize(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_R4G4_UNORM_PACK8:
        return 1;
    case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        return 2;
    case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        return 2;
    case VK_FORMAT_R5G6B5_UNORM_PACK16:
        return 2;
    case VK_FORMAT_B5G6R5_UNORM_PACK16:
        return 2;
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
        return 2;
    case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
        return 2;
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        return 2;
    case VK_FORMAT_R8_UNORM:
        return 1;
    case VK_FORMAT_R8_SNORM:
        return 1;
    case VK_FORMAT_R8_USCALED:
        return 1;
    case VK_FORMAT_R8_SSCALED:
        return 1;
    case VK_FORMAT_R8_UINT:
        return 1;
    case VK_FORMAT_R8_SINT:
        return 1;
    case VK_FORMAT_R8_SRGB:
        return 1;
    case VK_FORMAT_R8G8_UNORM:
        return 2;
    case VK_FORMAT_R8G8_SNORM:
        return 2;
    case VK_FORMAT_R8G8_USCALED:
        return 2;
    case VK_FORMAT_R8G8_SSCALED:
        return 2;
    case VK_FORMAT_R8G8_UINT:
        return 2;
    case VK_FORMAT_R8G8_SINT:
        return 2;
    case VK_FORMAT_R8G8_SRGB:
        return 2;
    case VK_FORMAT_R8G8B8_UNORM:
        return 3;
    case VK_FORMAT_R8G8B8_SNORM:
        return 3;
    case VK_FORMAT_R8G8B8_USCALED:
        return 3;
    case VK_FORMAT_R8G8B8_SSCALED:
        return 3;
    case VK_FORMAT_R8G8B8_UINT:
        return 3;
    case VK_FORMAT_R8G8B8_SINT:
        return 3;
    case VK_FORMAT_R8G8B8_SRGB:
        return 3;
    case VK_FORMAT_B8G8R8_UNORM:
        return 3;
    case VK_FORMAT_B8G8R8_SNORM:
        return 3;
    case VK_FORMAT_B8G8R8_USCALED:
        return 3;
    case VK_FORMAT_B8G8R8_SSCALED:
        return 3;
    case VK_FORMAT_B8G8R8_UINT:
        return 3;
    case VK_FORMAT_B8G8R8_SINT:
        return 3;
    case VK_FORMAT_B8G8R8_SRGB:
        return 3;
    case VK_FORMAT_R8G8B8A8_UNORM:
        return 4;
    case VK_FORMAT_R8G8B8A8_SNORM:
        return 4;
    case VK_FORMAT_R8G8B8A8_USCALED:
        return 4;
    case VK_FORMAT_R8G8B8A8_SSCALED:
        return 4;
    case VK_FORMAT_R8G8B8A8_UINT:
        return 4;
    case VK_FORMAT_R8G8B8A8_SINT:
        return 4;
    case VK_FORMAT_R8G8B8A8_SRGB:
        return 4;
    case VK_FORMAT_B8G8R8A8_UNORM:
        return 4;
    case VK_FORMAT_B8G8R8A8_SNORM:
        return 4;
    case VK_FORMAT_B8G8R8A8_USCALED:
        return 4;
    case VK_FORMAT_B8G8R8A8_SSCALED:
        return 4;
    case VK_FORMAT_B8G8R8A8_UINT:
        return 4;
    case VK_FORMAT_B8G8R8A8_SINT:
        return 4;
    case VK_FORMAT_B8G8R8A8_SRGB:
        return 4;
    case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        return 4;
    case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        return 4;
    case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
        return 4;
    case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
        return 4;
    case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        return 4;
    case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        return 4;
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        return 4;
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        return 4;
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
        return 4;
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
        return 4;
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
        return 4;
    case VK_FORMAT_A2R10G10B10_UINT_PACK32:
        return 4;
    case VK_FORMAT_A2R10G10B10_SINT_PACK32:
        return 4;
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        return 4;
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
        return 4;
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
        return 4;
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
        return 4;
    case VK_FORMAT_A2B10G10R10_UINT_PACK32:
        return 4;
    case VK_FORMAT_A2B10G10R10_SINT_PACK32:
        return 4;
    case VK_FORMAT_R16_UNORM:
        return 2;
    case VK_FORMAT_R16_SNORM:
        return 2;
    case VK_FORMAT_R16_USCALED:
        return 2;
    case VK_FORMAT_R16_SSCALED:
        return 2;
    case VK_FORMAT_R16_UINT:
        return 2;
    case VK_FORMAT_R16_SINT:
        return 2;
    case VK_FORMAT_R16_SFLOAT:
        return 2;
    case VK_FORMAT_R16G16_UNORM:
        return 4;
    case VK_FORMAT_R16G16_SNORM:
        return 4;
    case VK_FORMAT_R16G16_USCALED:
        return 4;
    case VK_FORMAT_R16G16_SSCALED:
        return 4;
    case VK_FORMAT_R16G16_UINT:
        return 4;
    case VK_FORMAT_R16G16_SINT:
        return 4;
    case VK_FORMAT_R16G16_SFLOAT:
        return 6;
    case VK_FORMAT_R16G16B16_UNORM:
        return 6;
    case VK_FORMAT_R16G16B16_SNORM:
        return 6;
    case VK_FORMAT_R16G16B16_USCALED:
        return 6;
    case VK_FORMAT_R16G16B16_SSCALED:
        return 6;
    case VK_FORMAT_R16G16B16_UINT:
        return 6;
    case VK_FORMAT_R16G16B16_SINT:
        return 6;
    case VK_FORMAT_R16G16B16_SFLOAT:
        return 6;
    case VK_FORMAT_R16G16B16A16_UNORM:
        return 8;
    case VK_FORMAT_R16G16B16A16_SNORM:
        return 8;
    case VK_FORMAT_R16G16B16A16_USCALED:
        return 8;
    case VK_FORMAT_R16G16B16A16_SSCALED:
        return 8;
    case VK_FORMAT_R16G16B16A16_UINT:
        return 8;
    case VK_FORMAT_R16G16B16A16_SINT:
        return 8;
    case VK_FORMAT_R16G16B16A16_SFLOAT:
        return 8;
    case VK_FORMAT_R32_UINT:
        return 4;
    case VK_FORMAT_R32_SINT:
        return 4;
    case VK_FORMAT_R32_SFLOAT:
        return 4;
    case VK_FORMAT_R32G32_UINT:
        return 8;
    case VK_FORMAT_R32G32_SINT:
        return 8;
    case VK_FORMAT_R32G32_SFLOAT:
        return 8;
    case VK_FORMAT_R32G32B32_UINT:
        return 12;
    case VK_FORMAT_R32G32B32_SINT:
        return 12;
    case VK_FORMAT_R32G32B32_SFLOAT:
        return 12;
    case VK_FORMAT_R32G32B32A32_UINT:
        return 16;
    case VK_FORMAT_R32G32B32A32_SINT:
        return 16;
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        return 16;
    case VK_FORMAT_R64_UINT:
        return 8;
    case VK_FORMAT_R64_SINT:
        return 8;
    case VK_FORMAT_R64_SFLOAT:
        return 8;
    case VK_FORMAT_R64G64_UINT:
        return 16;
    case VK_FORMAT_R64G64_SINT:
        return 16;
    case VK_FORMAT_R64G64_SFLOAT:
        return 16;
    case VK_FORMAT_R64G64B64_UINT:
        return 24;
    case VK_FORMAT_R64G64B64_SINT:
        return 24;
    case VK_FORMAT_R64G64B64_SFLOAT:
        return 24;
    case VK_FORMAT_R64G64B64A64_UINT:
        return 32;
    case VK_FORMAT_R64G64B64A64_SINT:
        return 32;
    case VK_FORMAT_R64G64B64A64_SFLOAT:
        return 32;
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        return 4;
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
        return 4;
    case VK_FORMAT_D16_UNORM:
        return 2;
    case VK_FORMAT_X8_D24_UNORM_PACK32:
        return 4;
    case VK_FORMAT_D32_SFLOAT:
        return 4;
    case VK_FORMAT_S8_UINT:
        return 1;
    case VK_FORMAT_D16_UNORM_S8_UINT:
        return 3;
    case VK_FORMAT_D24_UNORM_S8_UINT:
        return 4;
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return 4;
    default:
        return 0;
    }
}

inline VkImageLayout StateToImageLayout(EResourceState state)
{
    switch (state)
    {
    case EResourceState::Undefined:
        return VK_IMAGE_LAYOUT_UNDEFINED;
    case EResourceState::PreInitialized:
        return VK_IMAGE_LAYOUT_PREINITIALIZED;
    case EResourceState::General:
    case EResourceState::UnorderedAccess:
        return VK_IMAGE_LAYOUT_GENERAL;
    case EResourceState::RenderTarget:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case EResourceState::DepthRead:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    case EResourceState::DepthWrite:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case EResourceState::ShaderResource:
    case EResourceState::PixelShaderResource:
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case EResourceState::CopyDest:
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case EResourceState::CopySource:
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case EResourceState::Present:
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    default:
        throw std::runtime_error("Vulkan RHI resource state invalid");
    }
}

inline VkAccessFlags StateToAccessMask(EResourceState state)
{
    switch (state)
    {
    case EResourceState::Undefined:
    case EResourceState::PreInitialized:
        return 0;
    case EResourceState::General:
        return 0; // TODO
    case EResourceState::IndirectArg:
        return VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    case EResourceState::IndexBuffer:
        return VK_ACCESS_INDEX_READ_BIT;
    case EResourceState::VertexBuffer:
        return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    case EResourceState::ConstantBuffer:
        return VK_ACCESS_UNIFORM_READ_BIT;
    case EResourceState::RenderTarget:
        return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    case EResourceState::UnorderedAccess:
        return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    case EResourceState::DepthRead:
        return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    case EResourceState::DepthWrite:
        return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
            | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    case EResourceState::ShaderResource:
    case EResourceState::PixelShaderResource:
        return VK_ACCESS_SHADER_READ_BIT;
    case EResourceState::CopyDest:
        return VK_ACCESS_TRANSFER_WRITE_BIT;
    case EResourceState::CopySource:
        return VK_ACCESS_TRANSFER_READ_BIT;
    case EResourceState::Present:
        return 0;
    default:
        throw std::runtime_error("Vulkan RHI resource state invalid");
    }
}

inline VkPipelineStageFlags StateToShaderStageMask(EResourceState state, bool src)
{
    switch (state)
    {
    case EResourceState::Undefined:
    case EResourceState::PreInitialized:
        assert(src); // Transition to those states makes no sense
        return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    case EResourceState::General:
        return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    case EResourceState::IndirectArg:
        return VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
    case EResourceState::IndexBuffer:
    case EResourceState::VertexBuffer:
        return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    case EResourceState::ConstantBuffer:
    case EResourceState::UnorderedAccess:
    case EResourceState::ShaderResource:
        return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
            | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    case EResourceState::PixelShaderResource:
        return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    case EResourceState::RenderTarget:
        return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    case EResourceState::DepthRead:
    case EResourceState::DepthWrite:
        return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
            | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    case EResourceState::CopyDest:
    case EResourceState::CopySource:
        return VK_PIPELINE_STAGE_TRANSFER_BIT;
    case EResourceState::Present:
        return src ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    default:
        throw std::runtime_error("Vulkan RHI resource state invalid");
    }
}

}
