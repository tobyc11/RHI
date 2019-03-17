#pragma once
#include "PipelineStateDesc.h"
#include "ShaderModule.h"
#include <vector>

namespace RHI
{

enum class EPrimitiveTopology
{
    PointList = 0,
    LineList = 1,
    LineStrip = 2,
    TriangleList = 3,
    TriangleStrip = 4,
    TriangleFan = 5,
};

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

    const CVertexShaderInputBinding& GetVertexInputBinding() const { return VertexShaderInputBinding; }
    CVertexShaderInputBinding& GetVertexInputBinding() { return VertexShaderInputBinding; }

    sp<CShaderModule> GetVertexShader() const { return VertexShader; }
    void SetVertexShader(CShaderModule* value) { VertexShader = value; }

    sp<CShaderModule> GetPixelShader() const { return PixelShader; }
    void SetPixelShader(CShaderModule* value) { PixelShader = value; }

    const sp<CBuffer>& GetIndexBuffer() const { return IndexBuffer; }
    void SetIndexBuffer(CBuffer* value) { IndexBuffer = value; }

    const CPipelineArguments& GetPipelineArguments() const { return PipelineArgs; }
    CPipelineArguments& GetPipelineArguments() { return PipelineArgs; }

    bool IsIndexed = false;
    uint32_t IndexWidth = 4;

    uint32_t ElementCount = 0;
    uint32_t InstanceCount = 0; //Zero means not instanced
    uint32_t VertexOffset = 0;
    uint32_t IndexOffset = 0;
    uint32_t InstanceOffset = 0;

    //Those hash functions aren't used right now, but will be needed when we do pipeline caching
    std::size_t HashStaticStates() const
    {
        std::size_t result = 0;
        tc::hash_combine(result, Rasterizer);
        tc::hash_combine(result, DepthStencil);
        tc::hash_combine(result, Blend);
        tc::hash_combine(result, VertexShader);
        tc::hash_combine(result, PixelShader);
        return result;
    }

    bool StaticStatesEqual(const CDrawTemplate& rhs) const
    {
        return Rasterizer == rhs.Rasterizer &&
            DepthStencil == rhs.DepthStencil &&
            Blend == rhs.Blend &&
            VertexShader == rhs.VertexShader &&
            PixelShader == rhs.PixelShader;
    }

private:
    CRasterizerDesc Rasterizer;
    CDepthStencilDesc DepthStencil;
    CBlendDesc Blend;

    sp<CShaderModule> VertexShader;
    sp<CShaderModule> PixelShader;

    //Dynamic states
    std::vector<CViewportDesc> Viewports;
    std::vector<CRectDesc> Scissors;

    CVertexShaderInputBinding VertexShaderInputBinding;
    sp<CBuffer> IndexBuffer;
    CPipelineArguments PipelineArgs;
};

} /* namespace RHI */
