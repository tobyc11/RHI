#include "ShaderD3D11.h"
#include "BufferD3D11.h"
#include "ImageD3D11.h"
#include "SamplerD3D11.h"
#include "RHIException.h"
#include <CompileTimeHash.h>
#include <fstream>
#include <iostream>

namespace Nome::RHI
{

class IDeviceContextShaderRedir
{
public:
    virtual void SetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* pp) = 0;
    virtual void SetShaderResources(UINT StartSlot, UINT NumBuffers, ID3D11ShaderResourceView* const* pp) = 0;
    virtual void SetSamplers(UINT StartSlot, UINT NumBuffers, ID3D11SamplerState* const* pp) = 0;
};

class CVSRedir : public IDeviceContextShaderRedir
{
    ID3D11DeviceContext* Ctx;
public:
    CVSRedir(ID3D11DeviceContext* ctx) : Ctx(ctx) {}
    void SetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* pp) override
    {
        Ctx->VSSetConstantBuffers(StartSlot, NumBuffers, pp);
    }
    void SetShaderResources(UINT StartSlot, UINT NumBuffers, ID3D11ShaderResourceView* const* pp) override
    {
        Ctx->VSSetShaderResources(StartSlot, NumBuffers, pp);
    }
    void SetSamplers(UINT StartSlot, UINT NumBuffers, ID3D11SamplerState* const* pp) override
    {
        Ctx->VSSetSamplers(StartSlot, NumBuffers, pp);
    }
};

class CPSRedir : public IDeviceContextShaderRedir
{
    ID3D11DeviceContext* Ctx;
public:
    CPSRedir(ID3D11DeviceContext* ctx) : Ctx(ctx) {}
    void SetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* pp) override
    {
        Ctx->PSSetConstantBuffers(StartSlot, NumBuffers, pp);
    }
    void SetShaderResources(UINT StartSlot, UINT NumBuffers, ID3D11ShaderResourceView* const* pp) override
    {
        Ctx->PSSetShaderResources(StartSlot, NumBuffers, pp);
    }
    void SetSamplers(UINT StartSlot, UINT NumBuffers, ID3D11SamplerState* const* pp) override
    {
        Ctx->PSSetSamplers(StartSlot, NumBuffers, pp);
    }
};

void CPipelineParamMappingD3D11::BindArguments(const CPipelineArguments& args, ID3D11DeviceContext* ctx)
{
    //Local vars used by the switch statement
    sp<CBufferD3D11> buf;
    sp<CImageViewD3D11> view;
    sp<CSamplerD3D11> sampler;
    ID3D11Buffer* bufPtr;
    CVSRedir ctxWrapper(ctx);

    for (const auto& pair : args.Arguments)
    {
        uint32_t argId = pair.first;
        auto paramIter = Mappings.find(argId);
        if (paramIter == Mappings.end())
        {
            printf("Argument %d does not match any parameter.\n", argId);
            continue;
        }

        switch (pair.second.index())
        {
        //CBuffer
        case 0:
            buf = static_cast<CBufferD3D11*>(std::get<sp<CBuffer>>(pair.second).get());
            bufPtr = buf->GetD3D11Buffer();
            ctxWrapper.SetConstantBuffers(paramIter->second, 1, &bufPtr);
            break;
        //CImageView
        case 1:
            view = static_cast<CImageViewD3D11*>(std::get<sp<CImageView>>(pair.second).get());
            break;
        //CSampler
        case 2:
            sampler = static_cast<CSamplerD3D11*>(std::get<sp<CSampler>>(pair.second).get());
            break;
        }
    }
}

CShaderD3D11::CShaderD3D11(CShaderModule& fromSrc)
{
    //Read the shader template from disk
    std::ifstream ifs(fromSrc.SourcePath);
    std::string str;

    ifs.seekg(0, std::ios::end);
    str.reserve(ifs.tellg());
    ifs.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    // Disable optimizations to further improve shader debugging
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    //Compile vertex shader
    ID3DBlob* errorBlob;
    D3DCompile(str.c_str(), str.size(), fromSrc.SourcePath.c_str(), nullptr, nullptr,
        fromSrc.EntryPoint.c_str(), fromSrc.Target.c_str(), dwShaderFlags, 0, &CodeBlob, &errorBlob);
    if (errorBlob)
    {
        std::cout << "[" << fromSrc.Target << "] Shader compilation error:" << std::endl;
        std::cout << reinterpret_cast<const char*>(errorBlob->GetBufferPointer()) << std::endl;
        errorBlob->Release();
        throw CRHIException("Shader compilation failed.");
    }
}

CShaderD3D11::~CShaderD3D11()
{
    if (CodeBlob)
        CodeBlob->Release();
}

void CShaderD3D11::GenMappings()
{
    ID3D11ShaderReflection* reflector;
    D3DReflect(CodeBlob->GetBufferPointer(), CodeBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflector);
    if (reflector)
    {
        D3D11_SHADER_DESC shaderDesc;
        reflector->GetDesc(&shaderDesc);

        //Record constant buffer mappings
        for (uint32_t i = 0; i < shaderDesc.ConstantBuffers; i++)
        {
            //TODO: i is not necessarily the binding slot
            auto* cbufferReflector = reflector->GetConstantBufferByIndex(i);
            D3D11_SHADER_BUFFER_DESC cbufferDesc;
            cbufferReflector->GetDesc(&cbufferDesc);
            uint32_t paramId = tc::StrHash(cbufferDesc.Name);
            Mappings.Mappings[paramId] = i;
#ifdef _DEBUG
            printf("Constant Buffer %d(%s, %d)\n", paramId, cbufferDesc.Name, i);
#endif
        }

        //Record resource mappings
        for (uint32_t i = 0; i < shaderDesc.BoundResources; i++)
        {
            D3D11_SHADER_INPUT_BIND_DESC desc;
            reflector->GetResourceBindingDesc(i, &desc);
            uint32_t paramId = tc::StrHash(desc.Name);
            Mappings.Mappings[paramId] = desc.BindPoint;
#ifdef _DEBUG
            printf("Shader Resource %d(%s, %d)\n", paramId, desc.Name, desc.BindPoint);
#endif
        }
        reflector->Release();
    }
}

} /* namespace Nome::RHI */
