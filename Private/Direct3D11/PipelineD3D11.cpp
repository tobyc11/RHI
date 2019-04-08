#include "PipelineD3D11.h"
#include "DeviceD3D11.h"
#include "ShaderD3D11.h"
#include "StateCacheD3D11.h"

namespace RHI
{

CPipelineD3D11::CPipelineD3D11(CDeviceD3D11& p, const CPipelineDesc& desc)
    : Parent(p)
{
    HRESULT hr;

    ComPtr<ID3DBlob> VSCode;
    if (desc.VS)
    {
        auto impl = std::static_pointer_cast<CShaderModuleD3D11>(desc.VS);
        VS = impl->GetVS();
        VSDescriptorSetLayouts = impl->GetSPIRVCompiler()->GetLayoutRemaps();
        VSCode = impl->GetSPIRVCompiler()->GetOrCompileDXBC();
    }
    if (desc.PS)
    {
        auto impl = std::static_pointer_cast<CShaderModuleD3D11>(desc.PS);
        PS = impl->GetPS();
        PSDescriptorSetLayouts = impl->GetSPIRVCompiler()->GetLayoutRemaps();
    }

    if (!desc.VertexBindings.empty())
    {
        std::vector<D3D11_INPUT_ELEMENT_DESC> inputDesc;

        // Populate binding info
        std::map<uint32_t, bool> bindingToIsPerInstance;
        for (const auto& bindingDesc : desc.VertexBindings)
        {
            BindingToStride[bindingDesc.Binding] = bindingDesc.Stride;
            bindingToIsPerInstance[bindingDesc.Binding] = bindingDesc.bIsPerInstance;
        }

        for (const auto& a : desc.VertexAttributes)
        {
            bool isInst = bindingToIsPerInstance[a.Binding];
            D3D11_INPUT_ELEMENT_DESC elem {
                "TEXCOORD",
                a.Location,
                Convert(a.Format),
                a.Binding,
                a.Offset,
                isInst ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA,
                isInst ? 1u : 0u
            };
            inputDesc.push_back(elem);
        }

        hr = Parent.D3dDevice->CreateInputLayout(
            inputDesc.data(), (UINT)inputDesc.size(), VSCode->GetBufferPointer(),
            VSCode->GetBufferSize(), InputLayout.GetAddressOf());
        if (FAILED(hr))
            throw CRHIRuntimeError("D3dDevice->CreateInputLayout failed");
    }

    PrimitiveTopology = desc.PrimitiveTopology;
    RasterizerState = Parent.StateCache->FindOrCreate(*desc.RasterizerState);
    DepthStencilState = Parent.StateCache->FindOrCreate(*desc.DepthStencilState);
    BlendState = Parent.StateCache->FindOrCreate(*desc.BlendState);
}

CPipelineD3D11::~CPipelineD3D11()
{
    CSPIRVToHLSL::ReleaseLayoutRemaps(VSDescriptorSetLayouts);
    CSPIRVToHLSL::ReleaseLayoutRemaps(PSDescriptorSetLayouts);
}

}
