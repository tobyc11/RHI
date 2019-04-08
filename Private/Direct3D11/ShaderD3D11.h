#pragma once
#include "D3D11Platform.h"
#include "SPIRVToHLSL.h"
#include "ShaderModule.h"
#include <map>
#include <memory>
#include <unordered_map>
#include <variant>

namespace RHI
{

// Mapping from user-facing parameters to pipeline slots for a single shader stage
class CVSRedir;
class CPSRedir;
class CPipelineParamMappingD3D11
{
public:
    // Id to slot
    std::map<uint32_t, uint32_t> Mappings;

    template <typename TRedir>
    void BindArguments(const CPipelineArguments& args, ID3D11DeviceContext* ctx) const;
};

class CShaderD3D11
{
public:
    CShaderD3D11(CDeviceD3D11& p, CShaderModule& fromSrc);

    const CVertexShaderInputSignature& GetVSInputSignature() const;
    const CPipelineParamMappingD3D11& GetParamMappings() const { return Mappings; }

    ComPtr<ID3DBlob> GetCodeBlob() const;

    ID3D11VertexShader* GetVS() const;
    ID3D11PixelShader* GetPS() const;

private:
    void GenMappings();

    CDeviceD3D11& Parent;

    // Code
    ComPtr<ID3DBlob> CodeBlob;

    // Reflection data
    CVertexShaderInputSignature InputSig;
    CPipelineParamMappingD3D11 Mappings;

    mutable ComPtr<ID3D11VertexShader> VS;
    mutable ComPtr<ID3D11PixelShader> PS;
};

class CShaderCacheD3D11
{
public:
    CShaderCacheD3D11() = default;
    CShaderCacheD3D11(const CShaderCacheD3D11&) = delete;
    CShaderCacheD3D11(CShaderCacheD3D11&&) = delete;
    CShaderCacheD3D11& operator=(const CShaderCacheD3D11&) = delete;
    CShaderCacheD3D11& operator=(CShaderCacheD3D11&&) = delete;

    ~CShaderCacheD3D11()
    {
        for (auto pair : ShaderCache)
            delete pair.second;
    }

    CShaderD3D11* GetShader(const std::string& key) const
    {
        auto iter = ShaderCache.find(key);
        if (iter == ShaderCache.end())
            return nullptr;
        return iter->second;
    }

    void PutShader(const std::string& key, CShaderD3D11* shader) { ShaderCache[key] = shader; }

    // CShaderD3D11* LoadFromDisk(const std::string& key);
    // void SaveAllToDisk();

private:
    // Owns all those shaders also
    std::unordered_map<std::string, CShaderD3D11*> ShaderCache;
};

class CShaderModuleD3D11 : public CShaderModule
{
public:
    CShaderModuleD3D11(CDeviceD3D11& p, std::vector<uint32_t> spirvBinary);

    CSPIRVToHLSL* GetSPIRVCompiler() const { return SPIRVCompiler.get(); }
    ComPtr<ID3D11VertexShader> GetVS() const { return VS; }
    ComPtr<ID3D11PixelShader> GetPS() const { return PS; }

private:
    CDeviceD3D11& Parent;

    std::unique_ptr<CSPIRVToHLSL> SPIRVCompiler;

    ComPtr<ID3D11VertexShader> VS;
    ComPtr<ID3D11PixelShader> PS;
};

} /* namespace RHI */
