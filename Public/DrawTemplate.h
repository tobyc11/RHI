#pragma once
#include "PipelineStateDesc.h"
#include "ShaderModule.h"
#include <vector>

namespace Nome::RHI
{

class CDrawTemplate
{
public:
    const std::vector<CViewportDesc>& GetViewports() const { return Viewports; }
    void SetViewports(std::vector<CViewportDesc> value) { Viewports = std::move(value); }

    const std::vector<CRectDesc>& GetScissors() const { return Scissors; }
    void SetScissors(std::vector<CRectDesc> value) { Scissors = std::move(value); }

    const CRasterizerDesc& GetRasterizerDesc() const { return Rasterizer; }
    void SetRasterizerDesc(CRasterizerDesc value) { Rasterizer = value; }
    
    const CDepthStencilDesc& GetDepthStencilDesc() const { return DepthStencil; }
    void SetDepthStencilDesc(CDepthStencilDesc value) { DepthStencil = value; }
    
    const CBlendDesc& GetBlendDesc() const { return Blend; }
    void SetBlendDesc(CBlendDesc value) { Blend = value; }

private:
    std::vector<CViewportDesc> Viewports;
    std::vector<CRectDesc> Scissors;
    CRasterizerDesc Rasterizer;
    CDepthStencilDesc DepthStencil;
    CBlendDesc Blend;

    CVertexShaderInputBinding VertexShaderInputBinding;
    CPipelineArguments PipelineArgs;

    sp<CShaderModule> VertexShader;
    sp<CShaderModule> PixelShader;
};

} /* namespace Nome::RHI */
