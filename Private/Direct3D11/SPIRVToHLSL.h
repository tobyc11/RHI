#pragma once
#include "D3D11Platform.h"
#include <spirv_hlsl.hpp>

#include <Hash.h>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace RHI
{

struct CDescriptorSetRemap
{
    // Maps from binding to slot within the set
    std::unordered_map<uint32_t, uint32_t> Textures;
    std::unordered_map<uint32_t, uint32_t> Samplers;
    std::unordered_map<uint32_t, uint32_t> ConstantBuffers;

    friend std::size_t hash_value(const CDescriptorSetRemap& r)
    {
        std::size_t result = 0;
        for (const auto& pair : r.Textures)
        {
            tc::hash_combine(result, pair.first);
            tc::hash_combine(result, pair.second);
        }
        for (const auto& pair : r.Samplers)
        {
            tc::hash_combine(result, pair.first);
            tc::hash_combine(result, pair.second);
        }
        for (const auto& pair : r.ConstantBuffers)
        {
            tc::hash_combine(result, pair.first);
            tc::hash_combine(result, pair.second);
        }
        return result;
    }

    bool operator==(const CDescriptorSetRemap& r) const
    {
        return Textures.size() == r.Textures.size() && Samplers.size() == r.Samplers.size()
            && ConstantBuffers.size() == r.ConstantBuffers.size();
    }
};

struct CHashPointee
{
public:
    std::size_t operator()(const CDescriptorSetRemap* v) const { return hash_value(*v); }
};

struct CEqualPointee
{
public:
    bool operator()(const CDescriptorSetRemap* lhs, const CDescriptorSetRemap* rhs) const
    {
        return *lhs == *rhs;
    }
};

class CDescriptorSetCacheD3D11
{
public:
    const CDescriptorSetRemap* CreatePointerFromContent(const CDescriptorSetRemap& remap);
    void ReleasePointer(const CDescriptorSetRemap* pointer);

private:
    std::mutex CacheMutex;
    std::unordered_set<const CDescriptorSetRemap*, CHashPointee, CEqualPointee> CachedRemaps;
    std::unordered_map<const CDescriptorSetRemap*, size_t> RefCount;
};

class CSPIRVToHLSL
{
public:
    CSPIRVToHLSL(std::vector<uint32_t> spirvBinary);

    std::string Compile();
    ComPtr<ID3DBlob> GetOrCompileDXBC();
    std::unordered_map<uint32_t, const CDescriptorSetRemap*> GetLayoutRemaps();
    static void
    ReleaseLayoutRemaps(std::unordered_map<uint32_t, const CDescriptorSetRemap*>& remaps);
    char GetShaderStage();

private:
    spirv_cross::CompilerHLSL hlsl;
    char ShaderStage;
    std::unordered_map<uint32_t, CDescriptorSetRemap> DescSetRemaps;
    ComPtr<ID3DBlob> DXBCBlob;

    static CDescriptorSetCacheD3D11 DescriptorSetCache;
};

} /* namespace RHI */
