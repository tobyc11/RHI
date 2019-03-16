#pragma once
#include "Format.h"
#include "ImageView.h"
#include "Sampler.h"
#include "PipelineStateDesc.h"
#include "D3D11Platform.h"

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
D3D11_SHADER_RESOURCE_VIEW_DESC ConvertDescToSRV(const CImageViewDesc& desc);
D3D11_RENDER_TARGET_VIEW_DESC ConvertDescToRTV(const CImageViewDesc& desc);
D3D11_DEPTH_STENCIL_VIEW_DESC ConvertDescToDSV(const CImageViewDesc& desc);

} /* namespace RHI */
