#pragma once
#include "ConstantConverter.h"
#include "D3D11Platform.h"
#include "DeviceD3D11.h"
#include "PipelineStateDesc.h"
#include "RHIException.h"
#include <Hash.h>
#include <mutex>
#include <unordered_map>

namespace RHI
{

class CStateCacheD3D11
{
public:
    CStateCacheD3D11(CDeviceD3D11* parent)
        : Parent(parent) {};

    ComPtr<ID3D11RasterizerState> FindOrCreate(const CRasterizerDesc& inDesc)
    {
        std::unique_lock<std::mutex> lk(RasterizerMutex);

        auto iter = RasterizerCache.find(inDesc);
        if (iter == RasterizerCache.end())
        {
            ComPtr<ID3D11RasterizerState> rastState;
            // Translate RHI state block to D3D
            D3D11_RASTERIZER_DESC desc = {};
            desc.FillMode = Convert(inDesc.PolygonMode);
            desc.CullMode = Convert(inDesc.CullMode);
            desc.FrontCounterClockwise = inDesc.FrontFaceCCW;
            if (inDesc.DepthBiasEnable)
            {
                desc.DepthBias = static_cast<INT>(100.f * inDesc.DepthBiasConstantFactor);
                desc.DepthBiasClamp = inDesc.DepthBiasClamp;
                desc.SlopeScaledDepthBias = inDesc.DepthBiasSlopeFactor;
            }
            desc.DepthClipEnable = inDesc.DepthClampEnable;
            desc.ScissorEnable = false; // TODO
            desc.MultisampleEnable = false;
            desc.AntialiasedLineEnable = false;
            HRESULT hr = Parent->D3dDevice->CreateRasterizerState(&desc, rastState.GetAddressOf());
            if (!SUCCEEDED(hr))
                throw CRHIRuntimeError("Could not create ID3D11RasterizerState");
            RasterizerCache.emplace(inDesc, rastState);
            return rastState;
        }
        else
        {
            return iter->second;
        }
    }

    ComPtr<ID3D11DepthStencilState> FindOrCreate(const CDepthStencilDesc& inDesc)
    {
        std::unique_lock<std::mutex> lk(DepthStencilMutex);

        auto iter = DepthStencilCache.find(inDesc);
        if (iter == DepthStencilCache.end())
        {
            ComPtr<ID3D11DepthStencilState> stateBlock;
            // Translate RHI state block to D3D
            D3D11_DEPTH_STENCIL_DESC desc;
            desc.DepthEnable = inDesc.DepthEnable;
            desc.DepthWriteMask =
                inDesc.DepthWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
            desc.DepthFunc = Convert(inDesc.DepthCompareOp);
            desc.StencilEnable = inDesc.StencilEnable;
            desc.StencilReadMask = static_cast<UINT8>(inDesc.Front.CompareMask);
            desc.StencilWriteMask = static_cast<UINT8>(inDesc.Front.WriteMask);
            desc.FrontFace.StencilFailOp = Convert(inDesc.Front.FailOp);
            desc.FrontFace.StencilDepthFailOp = Convert(inDesc.Front.DepthFailOp);
            desc.FrontFace.StencilPassOp = Convert(inDesc.Front.PassOp);
            desc.FrontFace.StencilFunc = Convert(inDesc.Front.CompareOp);
            desc.BackFace.StencilFailOp = Convert(inDesc.Back.FailOp);
            desc.BackFace.StencilDepthFailOp = Convert(inDesc.Back.DepthFailOp);
            desc.BackFace.StencilPassOp = Convert(inDesc.Back.PassOp);
            desc.BackFace.StencilFunc = Convert(inDesc.Back.CompareOp);
            HRESULT hr =
                Parent->D3dDevice->CreateDepthStencilState(&desc, stateBlock.GetAddressOf());
            if (!SUCCEEDED(hr))
                throw CRHIRuntimeError("Could not create ID3D11DepthStencilState");
            DepthStencilCache.emplace(inDesc, stateBlock);
            return stateBlock;
        }
        else
        {
            return iter->second;
        }
    }

    ComPtr<ID3D11BlendState> FindOrCreate(const CBlendDesc& inDesc)
    {
        std::unique_lock<std::mutex> lk(BlendMutex);

        auto iter = BlendCache.find(inDesc);
        if (iter == BlendCache.end())
        {
            ComPtr<ID3D11BlendState> stateBlock;
            // Translate RHI state block to D3D
            D3D11_BLEND_DESC desc = {};
            desc.AlphaToCoverageEnable = false;
            desc.IndependentBlendEnable = inDesc.IndependentBlendEnable;
            auto convertOne = [](D3D11_RENDER_TARGET_BLEND_DESC& out,
                                 const CRenderTargetBlendDesc& in) {
                out.BlendEnable = in.BlendEnable;
                out.SrcBlend = Convert(in.SrcBlend);
                out.DestBlend = Convert(in.DestBlend);
                out.BlendOp = Convert(in.BlendOp);
                out.SrcBlendAlpha = Convert(in.SrcBlendAlpha);
                out.DestBlendAlpha = Convert(in.DestBlendAlpha);
                out.BlendOpAlpha = Convert(in.BlendOpAlpha);
                out.RenderTargetWriteMask = static_cast<UINT8>(in.RenderTargetWriteMask);
            };
            for (size_t i = 0; i < (desc.IndependentBlendEnable ? 8 : 1); i++)
                convertOne(desc.RenderTarget[i], inDesc.RenderTargets[i]);
            HRESULT hr = Parent->D3dDevice->CreateBlendState(&desc, stateBlock.GetAddressOf());
            if (!SUCCEEDED(hr))
                throw CRHIRuntimeError("Could not create ID3D11BlendState");
            BlendCache.emplace(inDesc, stateBlock);
            return stateBlock;
        }
        else
        {
            return iter->second;
        }
    }

private:
    CDeviceD3D11* Parent;
    std::mutex RasterizerMutex;
    std::unordered_map<CRasterizerDesc, ComPtr<ID3D11RasterizerState>, tc::hash<CRasterizerDesc>>
        RasterizerCache;
    std::mutex DepthStencilMutex;
    std::unordered_map<CDepthStencilDesc, ComPtr<ID3D11DepthStencilState>,
                       tc::hash<CDepthStencilDesc>>
        DepthStencilCache;
    std::mutex BlendMutex;
    std::unordered_map<CBlendDesc, ComPtr<ID3D11BlendState>, tc::hash<CBlendDesc>> BlendCache;
};

} /* namespace RHI */
