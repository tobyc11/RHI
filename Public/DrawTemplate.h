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

struct CBufferUpdateRequest
{
    sp<CBuffer> Buffer;
    uint32_t Size = 0;
    const void* Data = nullptr;
};

class CDrawTemplate
{
public:
    //Vertex Input
    const std::vector<CVertexInputAttributeDesc>& GetVertexAttributeDescs() const
    {
        return VertexAttributeDescs;
    }

    std::vector<CVertexInputAttributeDesc>& GetVertexAttributeDescs()
    {
        return VertexAttributeDescs;
    }

    const std::vector<CVertexInputBindingDesc>& GetVertexBindingDescs() const
    {
        return VertexBindingDescs;
    }

    std::vector<CVertexInputBindingDesc>& GetVertexBindingDescs()
    {
        return VertexBindingDescs;
    }

    //Input Assembly
    EPrimitiveTopology GetPrimitiveTopology() const { return PrimitiveTopology; }
    void SetPrimitiveTopology(EPrimitiveTopology value) { PrimitiveTopology = value; }

    //Rasterization
    const CRasterizerDesc& GetRasterizerDesc() const { return Rasterizer; }
    void SetRasterizerDesc(CRasterizerDesc value) { Rasterizer = value; }

    //DepthStencil
    const CDepthStencilDesc& GetDepthStencilDesc() const { return DepthStencil; }
    void SetDepthStencilDesc(CDepthStencilDesc value) { DepthStencil = value; }

    //ColorBlend
    const CBlendDesc& GetBlendDesc() const { return Blend; }
    void SetBlendDesc(CBlendDesc value) { Blend = value; }

    //Shader Stages
    sp<CShaderModule> GetVertexShader() const { return VertexShader; }
    void SetVertexShader(CShaderModule* value) { VertexShader = value; }

    sp<CShaderModule> GetPixelShader() const { return PixelShader; }
    void SetPixelShader(CShaderModule* value) { PixelShader = value; }

    //Dynamic stuff
    const CVertexInputs& GetVertexInputs() const { return VertexInputs; }
    CVertexInputs& GetVertexInputs() { return VertexInputs; }

    void SetIndexBuffer(CBuffer* value, uint32_t byteOffset, EFormat format)
    {
        IndexBuffer = value;
        IndexBufferOffset = byteOffset;
        IndexFormat = format;
    }

    sp<CBuffer> GetIndexBuffer() const { return IndexBuffer; }
    uint32_t GetIndexBufferOffset() const { return IndexBufferOffset; };
    EFormat GetIndexFormat() const { return IndexFormat; };

    const CPipelineArguments& GetPipelineArguments() const { return PipelineArgs; }
    CPipelineArguments& GetPipelineArguments() { return PipelineArgs; }

    const std::vector<CViewportDesc>& GetViewports() const { return Viewports; }
    void SetViewports(std::vector<CViewportDesc> value) { Viewports = std::move(value); }

    const std::vector<CRectDesc>& GetScissors() const { return Scissors; }
    void SetScissors(std::vector<CRectDesc> value) { Scissors = std::move(value); }

    const std::vector<CBufferUpdateRequest>& GetBufferUpdateReqs() const { return BufferUpdateReqs; }
    std::vector<CBufferUpdateRequest>& GetBufferUpdateReqs() { return BufferUpdateReqs; }

    uint32_t ElementCount = 0;
    uint32_t InstanceCount = 0; //Zero means not instanced
    uint32_t VertexOffset = 0;
    uint32_t IndexOffset = 0;
    uint32_t InstanceOffset = 0;

private:
    //Vertex Input
    std::vector<CVertexInputAttributeDesc> VertexAttributeDescs;
    std::vector<CVertexInputBindingDesc> VertexBindingDescs;

    //Input Assembly
    EPrimitiveTopology PrimitiveTopology = EPrimitiveTopology::TriangleList;

    //Rasterization
    CRasterizerDesc Rasterizer;
    
    //DepthStencil
    CDepthStencilDesc DepthStencil;
    
    //ColorBlend
    CBlendDesc Blend;

    //Shader Stages
    sp<CShaderModule> VertexShader;
    sp<CShaderModule> PixelShader;

    //Dynamic states
    CVertexInputs VertexInputs;
    sp<CBuffer> IndexBuffer;
    uint32_t IndexBufferOffset;
    EFormat IndexFormat;
    CPipelineArguments PipelineArgs;

    std::vector<CViewportDesc> Viewports;
    std::vector<CRectDesc> Scissors;

    std::vector<CBufferUpdateRequest> BufferUpdateReqs;
};

} /* namespace RHI */
