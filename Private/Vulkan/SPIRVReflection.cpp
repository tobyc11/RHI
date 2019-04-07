#include "SPIRVReflection.h"

// This file is pretty much copypasta from VEZ. I know, I am being lazy.
//
// Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

namespace RHI
{

static std::unordered_map<spirv_cross::SPIRType::BaseType, EBaseType> spirvTypeToVezBaseType = {
    { spirv_cross::SPIRType::Boolean, EBaseType::Bool },
    { spirv_cross::SPIRType::Char, EBaseType::Char },
    { spirv_cross::SPIRType::Int, EBaseType::Int },
    { spirv_cross::SPIRType::UInt, EBaseType::UInt },
    { spirv_cross::SPIRType::Half, EBaseType::Half },
    { spirv_cross::SPIRType::Float, EBaseType::Float },
    { spirv_cross::SPIRType::Double, EBaseType::Double },
    { spirv_cross::SPIRType::Struct, EBaseType::Struct },
};

VkShaderStageFlagBits SPIRVGetStage(spirv_cross::CompilerGLSL& compiler)
{
    for (const auto& ep : compiler.get_entry_points_and_stages())
    {
        switch (ep.execution_model)
        {
        case spv::ExecutionModelVertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case spv::ExecutionModelTessellationControl:
            return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case spv::ExecutionModelTessellationEvaluation:
            return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case spv::ExecutionModelGeometry:
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        case spv::ExecutionModelFragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        default:
            break;
        }
	}
    throw std::runtime_error("wtf");
}

bool SPIRVReflectResources(spirv_cross::CompilerGLSL& compiler, VkShaderStageFlagBits stage,
                           std::vector<CPipelineResource>& shaderResources)
{
    // Parse SPIRV binary.
    spirv_cross::CompilerGLSL::Options opts = compiler.get_common_options();
    opts.enable_420pack_extension = true;
    compiler.set_common_options(opts);

    // Reflect on all resource bindings.
    auto resources = compiler.get_shader_resources();

    // Extract per stage inputs.
    for (auto& resource : resources.stage_inputs)
    {
        const auto& spirType = compiler.get_type_from_variable(resource.id);

        CPipelineResource pipelineResource = {};
        pipelineResource.Stages = stage;
        pipelineResource.ResourceType = EPipelineResourceType::StageInput;
        pipelineResource.Access = VK_ACCESS_SHADER_READ_BIT;
        pipelineResource.Location = compiler.get_decoration(resource.id, spv::DecorationLocation);
        pipelineResource.VecSize = spirType.vecsize;
        pipelineResource.Columns = spirType.columns;
        pipelineResource.ArraySize = (spirType.array.size() == 0) ? 1 : spirType.array[0];

        auto it = spirvTypeToVezBaseType.find(spirType.basetype);
        if (it == spirvTypeToVezBaseType.end())
            continue;

        pipelineResource.BaseType = it->second;
        memcpy(pipelineResource.Name, resource.name.c_str(),
               std::min(sizeof(pipelineResource.Name), resource.name.length()));
        shaderResources.push_back(pipelineResource);
    }

    // Extract per stage outputs.
    for (auto& resource : resources.stage_outputs)
    {
        const auto& spirType = compiler.get_type_from_variable(resource.id);

        CPipelineResource pipelineResource = {};
        pipelineResource.Stages = stage;
        pipelineResource.ResourceType = EPipelineResourceType::StageOutput;
        pipelineResource.Access = VK_ACCESS_SHADER_WRITE_BIT;
        pipelineResource.Location = compiler.get_decoration(resource.id, spv::DecorationLocation);
        pipelineResource.VecSize = spirType.vecsize;
        pipelineResource.Columns = spirType.columns;
        pipelineResource.ArraySize = (spirType.array.size() == 0) ? 1 : spirType.array[0];

        auto it = spirvTypeToVezBaseType.find(spirType.basetype);
        if (it == spirvTypeToVezBaseType.end())
            continue;

        pipelineResource.BaseType = it->second;
        memcpy(pipelineResource.Name, resource.name.c_str(),
               std::min(sizeof(pipelineResource.Name), resource.name.length()));
        shaderResources.push_back(pipelineResource);
    }

    // Extract uniform buffers.
    for (auto& resource : resources.uniform_buffers)
    {
        const auto& spirType = compiler.get_type_from_variable(resource.id);

        CPipelineResource pipelineResource = {};
        pipelineResource.Stages = stage;
        pipelineResource.ResourceType = EPipelineResourceType::UniformBuffer;
        pipelineResource.Access = VK_ACCESS_UNIFORM_READ_BIT;
        pipelineResource.Set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        pipelineResource.Binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        pipelineResource.ArraySize = (spirType.array.size() == 0) ? 1 : spirType.array[0];
        pipelineResource.Size = static_cast<uint32_t>(compiler.get_declared_struct_size(spirType));
        memcpy(pipelineResource.Name, resource.name.c_str(),
               std::min(sizeof(pipelineResource.Name), resource.name.length()));
        shaderResources.push_back(pipelineResource);
    }

    // Extract storage buffers.
    for (auto& resource : resources.storage_buffers)
    {
        const auto& spirType = compiler.get_type_from_variable(resource.id);

        CPipelineResource pipelineResource = {};
        pipelineResource.Stages = stage;
        pipelineResource.ResourceType = EPipelineResourceType::StorageBuffer;
        pipelineResource.Access =
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT; // TODO: assume the worst
        pipelineResource.Set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        pipelineResource.Binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        pipelineResource.ArraySize = (spirType.array.size() == 0) ? 1 : spirType.array[0];
        pipelineResource.Size = static_cast<uint32_t>(compiler.get_declared_struct_size(spirType));
        memcpy(pipelineResource.Name, resource.name.c_str(),
               std::min(sizeof(pipelineResource.Name), resource.name.length()));
        shaderResources.push_back(pipelineResource);
    }

    // Extract separate samplers.
    for (auto& resource : resources.separate_samplers)
    {
        const auto& spirType = compiler.get_type_from_variable(resource.id);

        CPipelineResource pipelineResource = {};
        pipelineResource.Stages = stage;
        pipelineResource.ResourceType = EPipelineResourceType::SeparateSampler;
        pipelineResource.Access = VK_ACCESS_SHADER_READ_BIT;
        pipelineResource.Set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        pipelineResource.Binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        pipelineResource.ArraySize = (spirType.array.size() == 0) ? 1 : spirType.array[0];
        memcpy(pipelineResource.Name, resource.name.c_str(),
               std::min(sizeof(pipelineResource.Name), resource.name.length()));
        shaderResources.push_back(pipelineResource);
    }

    // Extract sampled images (combined sampler + image or texture buffers).
    for (auto& resource : resources.sampled_images)
    {
        const auto& spirType = compiler.get_type_from_variable(resource.id);

        CPipelineResource pipelineResource = {};
        pipelineResource.Stages = stage;
        pipelineResource.ResourceType = (spirType.image.dim == spv::Dim::DimBuffer)
            ? EPipelineResourceType::UniformTexelBuffer
            : EPipelineResourceType::CombinedImageSampler;
        pipelineResource.Access = VK_ACCESS_SHADER_READ_BIT;
        pipelineResource.Set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        pipelineResource.Binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        pipelineResource.ArraySize = (spirType.array.size() == 0) ? 1 : spirType.array[0];
        memcpy(pipelineResource.Name, resource.name.c_str(),
               std::min(sizeof(pipelineResource.Name), resource.name.length()));
        shaderResources.push_back(pipelineResource);
    }

    // Extract seperate images ('sampled' in vulkan terminology or no sampler attached).
    for (auto& resource : resources.separate_images)
    {
        const auto& spirType = compiler.get_type_from_variable(resource.id);

        CPipelineResource pipelineResource = {};
        pipelineResource.Stages = stage;
        pipelineResource.ResourceType = EPipelineResourceType::SeparateImage;
        pipelineResource.Access = VK_ACCESS_SHADER_READ_BIT;
        pipelineResource.Set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        pipelineResource.Binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        pipelineResource.ArraySize = (spirType.array.size() == 0) ? 1 : spirType.array[0];
        memcpy(pipelineResource.Name, resource.name.c_str(),
               std::min(sizeof(pipelineResource.Name), resource.name.length()));
        shaderResources.push_back(pipelineResource);
    }

    // Extract storage images.
    for (auto& resource : resources.storage_images)
    {
        auto nonReadable = compiler.get_decoration(resource.id, spv::DecorationNonReadable);
        auto nonWriteable = compiler.get_decoration(resource.id, spv::DecorationNonWritable);
        VkAccessFlags access = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        if (nonReadable)
            access = VK_ACCESS_SHADER_WRITE_BIT;
        else if (nonWriteable)
            access = VK_ACCESS_SHADER_READ_BIT;

        const auto& spirType = compiler.get_type_from_variable(resource.id);

        CPipelineResource pipelineResource = {};
        pipelineResource.Stages = stage;
        pipelineResource.ResourceType = (spirType.image.dim == spv::Dim::DimBuffer)
            ? EPipelineResourceType::StorageTexelBuffer
            : EPipelineResourceType::StorageImage;
        pipelineResource.Access = access;
        pipelineResource.Set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        pipelineResource.Binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        pipelineResource.ArraySize = (spirType.array.size() == 0) ? 1 : spirType.array[0];
        memcpy(pipelineResource.Name, resource.name.c_str(),
               std::min(sizeof(pipelineResource.Name), resource.name.length()));
        shaderResources.push_back(pipelineResource);
    }

    // Extract subpass inputs.
    for (auto& resource : resources.subpass_inputs)
    {
        CPipelineResource pipelineResource = {};
        pipelineResource.ResourceType = EPipelineResourceType::SubpassInput;
        pipelineResource.Stages = VK_SHADER_STAGE_FRAGMENT_BIT;
        pipelineResource.Access = VK_ACCESS_SHADER_READ_BIT;
        pipelineResource.InputAttachmentIndex =
            compiler.get_decoration(resource.id, spv::DecorationInputAttachmentIndex);
        pipelineResource.Set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        pipelineResource.Binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        pipelineResource.ArraySize = 1;
        memcpy(pipelineResource.Name, resource.name.c_str(),
               std::min(sizeof(pipelineResource.Name), resource.name.length()));
        shaderResources.push_back(pipelineResource);
    }

    // Extract push constants.
    for (auto& resource : resources.push_constant_buffers)
    {
        const auto& spirType = compiler.get_type_from_variable(resource.id);

        // Get the start offset of the push constant buffer since this will differ between shader
        // stages.
        uint32_t offset = ~0U;
        for (auto i = 0U; i < spirType.member_types.size(); ++i)
        {
            auto memberType = compiler.get_type(spirType.member_types[i]);
            offset = std::min(
                offset, compiler.get_member_decoration(spirType.self, i, spv::DecorationOffset));
        }

        CPipelineResource pipelineResource = {};
        pipelineResource.Stages = stage;
        pipelineResource.ResourceType = EPipelineResourceType::PushConstantBuffer;
        pipelineResource.Access = VK_ACCESS_SHADER_READ_BIT;
        pipelineResource.Offset = offset;
        pipelineResource.Size = static_cast<uint32_t>(compiler.get_declared_struct_size(spirType));
        memcpy(pipelineResource.Name, resource.name.c_str(),
               std::min(sizeof(pipelineResource.Name), resource.name.length()));
        shaderResources.push_back(pipelineResource);
    }

    return true;
}

}
