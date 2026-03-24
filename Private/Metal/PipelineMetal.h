#pragma once
#include "MtlCommon.h"
#include "Pipeline.h"
#include "SPIRVToMSL.h"

namespace RHI
{

class CPipelineMetal : public CPipeline
{
public:
    typedef std::shared_ptr<CPipelineMetal> Ref;

    CPipelineMetal(CDeviceMetal& parent, const CPipelineDesc& desc);
    CPipelineMetal(CDeviceMetal& parent, const CComputePipelineDesc& desc);
    ~CPipelineMetal() override;

    bool IsCompute() const { return bIsCompute; }
    id GetRenderPipelineState() const { return RenderPSO; }
    id GetComputePipelineState() const { return ComputePSO; }
    id GetDepthStencilState() const { return DepthStencilState; }

    MTLCullMode GetCullMode() const { return CullMode; }
    MTLTriangleFillMode GetFillMode() const { return FillMode; }
    MTLWinding GetWinding() const { return Winding; }
    bool GetDepthBiasEnable() const { return DepthBiasEnable; }
    float GetDepthBiasConstant() const { return DepthBiasConstant; }
    float GetDepthBiasSlope() const { return DepthBiasSlope; }
    float GetDepthBiasClamp() const { return DepthBiasClamp; }
    MTLPrimitiveType GetPrimitiveType() const { return PrimitiveType; }
    const CMSLBindingRemap& GetVSRemap() const { return VSRemap; }
    const CMSLBindingRemap& GetPSRemap() const { return PSRemap; }
    const CMSLBindingRemap& GetCSRemap() const { return CSRemap; }

private:
    bool bIsCompute = false;

    id RenderPSO = nil;
    id ComputePSO = nil;
    id DepthStencilState = nil;

    MTLCullMode CullMode = MTLCullModeBack;
    MTLTriangleFillMode FillMode = MTLTriangleFillModeFill;
    MTLWinding Winding = MTLWindingCounterClockwise;
    bool DepthBiasEnable = false;
    float DepthBiasConstant = 0.0f;
    float DepthBiasSlope = 0.0f;
    float DepthBiasClamp = 0.0f;
    MTLPrimitiveType PrimitiveType = {};

    CMSLBindingRemap VSRemap;
    CMSLBindingRemap PSRemap;
    CMSLBindingRemap CSRemap;
};

} /* namespace RHI */
