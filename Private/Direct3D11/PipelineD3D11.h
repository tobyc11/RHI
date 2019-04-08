#pragma once
#include "D3D11Platform.h"
#include "Pipeline.h"
#include "SPIRVToHLSL.h"

namespace RHI
{

class CPipelineD3D11 : public CPipeline
{
public:
    typedef std::shared_ptr<CPipelineD3D11> Ref;

    CPipelineD3D11(CDeviceD3D11& p, const CPipelineDesc& desc);
    ~CPipelineD3D11() override;

	//Not private for convineance, don't forget to update CContextD3D11 as well
    std::map<uint32_t, uint32_t> BindingToStride;
    ComPtr<ID3D11InputLayout> InputLayout;
    EPrimitiveTopology PrimitiveTopology;
    ComPtr<ID3D11RasterizerState> RasterizerState;
    ComPtr<ID3D11DepthStencilState> DepthStencilState;
    ComPtr<ID3D11BlendState> BlendState;

    ComPtr<ID3D11VertexShader> VS;
    std::unordered_map<uint32_t, const CDescriptorSetRemap*> VSDescriptorSetLayouts;
    ComPtr<ID3D11PixelShader> PS;
    std::unordered_map<uint32_t, const CDescriptorSetRemap*> PSDescriptorSetLayouts;

private:
    CDeviceD3D11& Parent;
};

}
