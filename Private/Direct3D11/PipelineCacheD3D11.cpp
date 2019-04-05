#include "PipelineCacheD3D11.h"
#include "ConstantConverter.h"
#include "StateCacheD3D11.h"
#include "RHIException.h"

#include <map>

namespace RHI
{

CPipelineStates CPipelineCacheD3D11::CreatePipelineStates(const CDrawTemplate& dt)
{
    CPipelineStatesD3D11* result = new CPipelineStatesD3D11();
    HRESULT hr;

    //Check the shaders first
    auto vs = dt.GetVertexShader();
    if (vs)
    {
        std::string key = vs->GetShaderCacheKey();
        if (!(result->VertexShader = DeviceImpl.ShaderCache->GetShader(key)))
        {
            result->VertexShader = new CShaderD3D11(*vs);
            result->VertexShader->GetVS(); //Triggers CreateVertexShader
            DeviceImpl.ShaderCache->PutShader(key, result->VertexShader);
        }

        if (dt.GetVertexBindingDescs().size())
        {
            //Do vertex input layout
            std::vector<D3D11_INPUT_ELEMENT_DESC> inputDesc;

            //Populate binding info
            std::map<uint32_t, bool> bindingToIsPerInstance;
            for (const auto& bindingDesc : dt.GetVertexBindingDescs())
            {
                result->BindingToStride[bindingDesc.Binding] = bindingDesc.Stride;
                bindingToIsPerInstance[bindingDesc.Binding] = bindingDesc.bIsPerInstance;
            }

            for (const auto& a : dt.GetVertexAttributeDescs())
            {
                bool isInst = bindingToIsPerInstance[a.Binding];
                D3D11_INPUT_ELEMENT_DESC elem{ "ATTRIBUTE", a.Location, Convert(a.Format), a.Binding, a.Offset,
                    isInst ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA, isInst ? 1u : 0u };
                inputDesc.push_back(elem);
            }

            auto b = result->VertexShader->GetCodeBlob();
            hr = DeviceImpl.D3dDevice->CreateInputLayout(inputDesc.data(), (UINT)inputDesc.size(),
                b->GetBufferPointer(), b->GetBufferSize(), result->InputLayout.GetAddressOf());
            if (FAILED(hr))
                throw CRHIRuntimeError("D3dDevice->CreateInputLayout failed");
        }
    }

    result->PrimitiveTopology = dt.GetPrimitiveTopology();

    auto ps = dt.GetPixelShader();
    if (ps)
    {
        std::string key = ps->GetShaderCacheKey();
        if (!(result->PixelShader = DeviceImpl.ShaderCache->GetShader(key)))
        {
            result->PixelShader = new CShaderD3D11(*ps);
            result->PixelShader->GetPS(); //Triggers CreatePixelShader
            DeviceImpl.ShaderCache->PutShader(key, result->PixelShader);
        }
    }

    result->RasterizerState = DeviceImpl.StateCache->FindOrCreate(dt.GetRasterizerDesc());
    result->DepthStencilState = DeviceImpl.StateCache->FindOrCreate(dt.GetDepthStencilDesc());
    result->BlendState = DeviceImpl.StateCache->FindOrCreate(dt.GetBlendDesc());

    //static_assert(sizeof(CViewportDesc) == sizeof(D3D11_VIEWPORT), "D3D11 viewport and CViewportDesc incompatible.");
    //for (const auto& desc : dt.GetViewports())
    //    result->Viewports.push_back(reinterpret_cast<const D3D11_VIEWPORT&>(desc));
    //static_assert(sizeof(CRect2D) == sizeof(D3D11_RECT), "D3D11 rect and CRect2Ds incompatible.");
    //for (const auto& desc : dt.GetScissors())
    //    result->Scissors.push_back(reinterpret_cast<const D3D11_RECT&>(desc));

    return result;
}

void CPipelineCacheD3D11::DestroyPipelineStates(CPipelineStates pso)
{
    delete static_cast<CPipelineStatesD3D11*>(pso);
}

}
