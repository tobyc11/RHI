#include "SPIRVToMSL.h"
#include <spirv_msl.hpp>
#include <unordered_map>

namespace RHI
{

static std::unordered_map<spirv_cross::SPIRType::BaseType, EBaseType> spirvTypeToBaseType = {
    { spirv_cross::SPIRType::Boolean, EBaseType::Bool },
    { spirv_cross::SPIRType::Char, EBaseType::Char },
    { spirv_cross::SPIRType::Int, EBaseType::Int },
    { spirv_cross::SPIRType::UInt, EBaseType::UInt },
    { spirv_cross::SPIRType::Half, EBaseType::Half },
    { spirv_cross::SPIRType::Float, EBaseType::Float },
    { spirv_cross::SPIRType::Double, EBaseType::Double },
    { spirv_cross::SPIRType::Struct, EBaseType::Struct },
};

static EShaderStageFlags GetStageFromSPIRV(spirv_cross::CompilerMSL& compiler)
{
    for (const auto& ep : compiler.get_entry_points_and_stages())
    {
        switch (ep.execution_model)
        {
        case spv::ExecutionModelVertex: return EShaderStageFlags::Vertex;
        case spv::ExecutionModelTessellationControl: return EShaderStageFlags::TessellationControl;
        case spv::ExecutionModelTessellationEvaluation:
            return EShaderStageFlags::TessellationEvaluation;
        case spv::ExecutionModelGeometry: return EShaderStageFlags::Geometry;
        case spv::ExecutionModelFragment: return EShaderStageFlags::Pixel;
        case spv::ExecutionModelGLCompute: return EShaderStageFlags::Compute;
        default: break;
        }
    }
    throw std::runtime_error("Could not determine shader stage from SPIRV");
}

static void ReflectResources(spirv_cross::CompilerMSL& compiler, EShaderStageFlags stage,
                             std::vector<CPipelineResource>& resources)
{
    auto shaderResources = compiler.get_shader_resources();

    auto reflect = [&](const spirv_cross::SmallVector<spirv_cross::Resource>& list,
                       EPipelineResourceType type, uint32_t access) {
        for (auto& res : list)
        {
            const auto& spirType = compiler.get_type_from_variable(res.id);

            CPipelineResource pr = {};
            pr.Stages = stage;
            pr.ResourceType = type;
            pr.Access = access;
            pr.Set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
            pr.Binding = compiler.get_decoration(res.id, spv::DecorationBinding);
            pr.Location = compiler.get_decoration(res.id, spv::DecorationLocation);
            pr.VecSize = spirType.vecsize;
            pr.Columns = spirType.columns;
            pr.ArraySize = (spirType.array.size() == 0) ? 1 : spirType.array[0];

            auto it = spirvTypeToBaseType.find(spirType.basetype);
            if (it != spirvTypeToBaseType.end())
                pr.BaseType = it->second;

            if (type == EPipelineResourceType::UniformBuffer ||
                type == EPipelineResourceType::StorageBuffer)
                pr.Size = static_cast<uint32_t>(compiler.get_declared_struct_size(spirType));

            if (type == EPipelineResourceType::SubpassInput)
                pr.InputAttachmentIndex =
                    compiler.get_decoration(res.id, spv::DecorationInputAttachmentIndex);

            memcpy(pr.Name, res.name.c_str(),
                   std::min(sizeof(pr.Name), res.name.length()));
            resources.push_back(pr);
        }
    };

    reflect(shaderResources.stage_inputs, EPipelineResourceType::StageInput, 1);
    reflect(shaderResources.stage_outputs, EPipelineResourceType::StageOutput, 2);
    reflect(shaderResources.uniform_buffers, EPipelineResourceType::UniformBuffer, 1);
    reflect(shaderResources.storage_buffers, EPipelineResourceType::StorageBuffer, 3);
    reflect(shaderResources.separate_samplers, EPipelineResourceType::SeparateSampler, 1);
    reflect(shaderResources.sampled_images, EPipelineResourceType::CombinedImageSampler, 1);
    reflect(shaderResources.separate_images, EPipelineResourceType::SeparateImage, 1);
    reflect(shaderResources.storage_images, EPipelineResourceType::StorageImage, 3);
    reflect(shaderResources.subpass_inputs, EPipelineResourceType::SubpassInput, 1);

    for (auto& res : shaderResources.push_constant_buffers)
    {
        const auto& spirType = compiler.get_type_from_variable(res.id);

        uint32_t offset = ~0U;
        for (auto i = 0U; i < spirType.member_types.size(); ++i)
        {
            offset = std::min(
                offset, compiler.get_member_decoration(spirType.self, i, spv::DecorationOffset));
        }

        CPipelineResource pr = {};
        pr.Stages = stage;
        pr.ResourceType = EPipelineResourceType::PushConstantBuffer;
        pr.Access = 1;
        pr.Offset = offset;
        pr.Size = static_cast<uint32_t>(compiler.get_declared_struct_size(spirType));
        memcpy(pr.Name, res.name.c_str(),
               std::min(sizeof(pr.Name), res.name.length()));
        resources.push_back(pr);
    }
}

CMSLCompileResult CompileSPIRVToMSL(const uint32_t* spirvCode, size_t wordCount)
{
    CMSLCompileResult result;

    std::vector<uint32_t> spirv(spirvCode, spirvCode + wordCount);
    spirv_cross::CompilerMSL compiler(std::move(spirv));

    spirv_cross::CompilerMSL::Options mslOptions;
    mslOptions.set_msl_version(2, 1);
    mslOptions.argument_buffers = false;
    mslOptions.force_active_argument_buffer_resources = false;
    compiler.set_msl_options(mslOptions);

    spirv_cross::CompilerGLSL::Options commonOptions;
    commonOptions.vertex.flip_vert_y = true;
    compiler.set_common_options(commonOptions);

    EShaderStageFlags stage = GetStageFromSPIRV(compiler);

    ReflectResources(compiler, stage, result.Resources);

    result.Source = compiler.compile();

    // Extract the binding remap that SPIRV-Cross computed
    auto shaderResources = compiler.get_shader_resources();

    auto extractRemap = [&](const spirv_cross::SmallVector<spirv_cross::Resource>& list,
                            bool isBuffer, bool isTexture, bool isSampler) {
        for (auto& res : list)
        {
            uint32_t set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
            uint32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);
            auto key = std::make_pair(set, binding);

            auto mslBinding = compiler.get_automatic_msl_resource_binding(res.id);
            if (mslBinding != uint32_t(-1))
            {
                if (isBuffer)
                    result.Remap.BufferBindings[key] = mslBinding;
                if (isTexture)
                    result.Remap.TextureBindings[key] = mslBinding;
                if (isSampler)
                    result.Remap.SamplerBindings[key] = mslBinding;
            }

            // Combined image sampler: extract secondary sampler binding
            if (isSampler)
            {
                auto mslSamplerBinding =
                    compiler.get_automatic_msl_resource_binding_secondary(res.id);
                if (mslSamplerBinding != uint32_t(-1))
                    result.Remap.SamplerBindings[key] = mslSamplerBinding;
            }
        }
    };

    extractRemap(shaderResources.uniform_buffers, true, false, false);
    extractRemap(shaderResources.storage_buffers, true, false, false);
    extractRemap(shaderResources.separate_images, false, true, false);
    extractRemap(shaderResources.storage_images, false, true, false);
    extractRemap(shaderResources.separate_samplers, false, false, true);
    extractRemap(shaderResources.sampled_images, false, true, true);
    extractRemap(shaderResources.subpass_inputs, false, true, false);

    return result;
}

} /* namespace RHI */
