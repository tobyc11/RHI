#include "SPIRVToHLSL.h"

namespace RHI
{

const CDescriptorSetRemap*
CDescriptorSetCacheD3D11::CreatePointerFromContent(const CDescriptorSetRemap& remap)
{
    std::unique_lock<std::mutex> lk(CacheMutex);

    auto iter = CachedRemaps.find(&remap);
    if (iter == CachedRemaps.end())
    {
        auto* alloc = new CDescriptorSetRemap(remap);
        CachedRemaps.insert(alloc);
        RefCount.emplace(alloc, 1);
        return alloc;
    }
    RefCount[*iter]++;
    return *iter;
}

void CDescriptorSetCacheD3D11::ReleasePointer(const CDescriptorSetRemap* pointer)
{
    std::unique_lock<std::mutex> lk(CacheMutex);
    RefCount[pointer]--;
    if (RefCount[pointer] == 0)
    {
        RefCount.erase(pointer);
        CachedRemaps.erase(pointer);
        delete pointer;
    }
}

CSPIRVToHLSL::CSPIRVToHLSL(std::vector<uint32_t> spirvBinary)
    : hlsl(std::move(spirvBinary))
{
    spirv_cross::CompilerHLSL::Options options;
    options.point_coord_compat = false;
    options.point_size_compat = false;
    options.support_nonzero_base_vertex_base_instance = false;
    options.shader_model = 40;
    hlsl.set_hlsl_options(options);

    spirv_cross::CompilerGLSL::Options commonOptions;
    commonOptions.vertex.flip_vert_y = true;
    hlsl.set_common_options(commonOptions);
}

std::string CSPIRVToHLSL::Compile()
{
    spirv_cross::ShaderResources resources = hlsl.get_shader_resources();

    // Resource remapping
    for (auto& resource : resources.separate_images)
    {
        unsigned set = hlsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
        unsigned binding = hlsl.get_decoration(resource.id, spv::DecorationBinding);
        uint32_t slot = set * 32;

        auto remap = DescSetRemaps.find(set);
        if (remap == DescSetRemaps.end())
        {
            CDescriptorSetRemap setRemap;
            setRemap.Textures.emplace(binding, slot);
            DescSetRemaps.emplace(set, setRemap);
        }
        else
        {
            slot += static_cast<uint32_t>(remap->second.Textures.size());
            remap->second.Textures.emplace(binding, slot);
        }

        hlsl.set_decoration(resource.id, spv::DecorationBinding, slot);
    }

    for (auto& resource : resources.separate_samplers)
    {
        unsigned set = hlsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
        unsigned binding = hlsl.get_decoration(resource.id, spv::DecorationBinding);
        uint32_t slot = set * 4;

        auto remap = DescSetRemaps.find(set);
        if (remap == DescSetRemaps.end())
        {
            CDescriptorSetRemap setRemap;
            setRemap.Samplers.emplace(binding, slot);
            DescSetRemaps.emplace(set, setRemap);
        }
        else
        {
            slot += static_cast<uint32_t>(remap->second.Samplers.size());
            remap->second.Samplers.emplace(binding, slot);
        }

        hlsl.set_decoration(resource.id, spv::DecorationBinding, slot);
    }

    for (auto& resource : resources.uniform_buffers)
    {
        unsigned set = hlsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
        unsigned binding = hlsl.get_decoration(resource.id, spv::DecorationBinding);
        uint32_t slot = set * 3;

        auto remap = DescSetRemaps.find(set);
        if (remap == DescSetRemaps.end())
        {
            CDescriptorSetRemap setRemap;
            setRemap.ConstantBuffers.emplace(binding, slot);
            DescSetRemaps.emplace(set, setRemap);
        }
        else
        {
            slot += static_cast<uint32_t>(remap->second.ConstantBuffers.size());
            remap->second.ConstantBuffers.emplace(binding, slot);
        }

        hlsl.set_decoration(resource.id, spv::DecorationBinding, slot);
    }

    return hlsl.compile();
}

ComPtr<ID3DBlob> CSPIRVToHLSL::GetOrCompileDXBC()
{
    auto entryPoints = hlsl.get_entry_points_and_stages();
    if (entryPoints.size() > 1)
        for (const auto& e : entryPoints)
            printf("[CSPIRVToHLSL] Entry point: %s, Stage: %d\n", e.name.c_str(),
                   e.execution_model);

    std::string shaderModel = "xs_5_0";
    switch (entryPoints[0].execution_model)
    {
    case spv::ExecutionModelVertex:
        ShaderStage = 'v';
        break;
    case spv::ExecutionModelTessellationControl:
        ShaderStage = 'h';
        break;
    case spv::ExecutionModelTessellationEvaluation:
        ShaderStage = 'd';
        break;
    case spv::ExecutionModelGeometry:
        ShaderStage = 'g';
        break;
    case spv::ExecutionModelFragment:
        ShaderStage = 'p';
        break;
    case spv::ExecutionModelGLCompute:
    case spv::ExecutionModelKernel:
        ShaderStage = 'c';
        break;
    default:
        break;
    }
    shaderModel[0] = ShaderStage;

    ComPtr<ID3DBlob> Code, ErrorMsgs;
    std::string hlslSrc = Compile();
    D3DCompile(hlslSrc.data(), hlslSrc.size(), nullptr, nullptr, nullptr,
               entryPoints[0].name.c_str(), shaderModel.c_str(), 0, 0, Code.GetAddressOf(),
               ErrorMsgs.GetAddressOf());

    if (ErrorMsgs)
    {
        printf("%s\n", static_cast<char*>(ErrorMsgs->GetBufferPointer()));
    }
    return DXBCBlob = Code;
}

std::unordered_map<uint32_t, const CDescriptorSetRemap*> CSPIRVToHLSL::GetLayoutRemaps()
{
    std::unordered_map<uint32_t, const CDescriptorSetRemap*> result;
    for (const auto& pair : DescSetRemaps)
        result.emplace(pair.first, DescriptorSetCache.CreatePointerFromContent(pair.second));
    return result;
}

void CSPIRVToHLSL::ReleaseLayoutRemaps(
    std::unordered_map<uint32_t, const CDescriptorSetRemap*>& remaps)
{
    for (auto& pair : remaps)
        DescriptorSetCache.ReleasePointer(pair.second);
    remaps.clear();
}

char CSPIRVToHLSL::GetShaderStage() { return ShaderStage; }

CDescriptorSetCacheD3D11 CSPIRVToHLSL::DescriptorSetCache;

} /* namespace RHI */
