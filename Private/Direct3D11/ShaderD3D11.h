#pragma once
#include "ShaderModule.h"
#include "D3D11Platform.h"
#include <map>
#include <variant>

namespace Nome::RHI
{

//Mapping from user-facing parameters to pipeline slots for a single shader stage
class CPipelineParamMappingD3D11
{
public:
    //Id to slot
    std::map<uint32_t, uint32_t> Mappings;

    void BindArguments(const CPipelineArguments& args, ID3D11DeviceContext* ctx);
};

class CShaderD3D11
{
public:
    CShaderD3D11(CShaderModule& fromSrc);
    ~CShaderD3D11();

private:
    void GenMappings();

    ID3DBlob* CodeBlob = nullptr;

    CPipelineParamMappingD3D11 Mappings;
};

class CShaderCacheD3D11
{
public:
};

} /* namespace Nome::RHI */
