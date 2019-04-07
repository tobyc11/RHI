#include "PipelineVk.h"
#include "DeviceVk.h"
#include "RenderPassVk.h"
#include "VkHelpers.h"

namespace RHI
{

static VkPrimitiveTopology VkCast(EPrimitiveTopology r)
{
    switch (r)
    {
    case RHI::EPrimitiveTopology::PointList:
    default:
        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case RHI::EPrimitiveTopology::LineList:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case RHI::EPrimitiveTopology::LineStrip:
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case RHI::EPrimitiveTopology::TriangleList:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case RHI::EPrimitiveTopology::TriangleStrip:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case RHI::EPrimitiveTopology::TriangleFan:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    }
}

static VkPolygonMode VkCast(EPolygonMode r)
{
    switch (r)
    {
    case RHI::EPolygonMode::Fill:
    default:
        return VK_POLYGON_MODE_FILL;
    case RHI::EPolygonMode::Wireframe:
        return VK_POLYGON_MODE_LINE;
    }
}

static VkCullModeFlagBits VkCast(ECullModeFlags f)
{
    VkFlags result = VK_CULL_MODE_NONE;
    if (Any(f, ECullModeFlags::Front))
        result |= VK_CULL_MODE_FRONT_BIT;
    if (Any(f, ECullModeFlags::Back))
        result |= VK_CULL_MODE_BACK_BIT;
    return static_cast<VkCullModeFlagBits>(result);
}

static VkBlendFactor VkCast(EBlendMode r)
{
    switch (r)
    {
    case RHI::EBlendMode::Zero:
    default:
        return VK_BLEND_FACTOR_ZERO;
    case RHI::EBlendMode::One:
        return VK_BLEND_FACTOR_ONE;
    case RHI::EBlendMode::SrcColor:
        return VK_BLEND_FACTOR_SRC_COLOR;
    case RHI::EBlendMode::OneMinusSrcColor:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case RHI::EBlendMode::SrcAlpha:
        return VK_BLEND_FACTOR_SRC_ALPHA;
    case RHI::EBlendMode::OneMinusSrcAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case RHI::EBlendMode::DstAlpha:
        return VK_BLEND_FACTOR_DST_ALPHA;
    case RHI::EBlendMode::OneMinusDstAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case RHI::EBlendMode::DstColor:
        return VK_BLEND_FACTOR_DST_COLOR;
    case RHI::EBlendMode::OneMinusDstColor:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case RHI::EBlendMode::SrcAlphaSaturate:
        return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    }
}

static VkBlendOp VkCast(EBlendOp r)
{
    switch (r)
    {
    case RHI::EBlendOp::Add:
    default:
        return VK_BLEND_OP_ADD;
    case RHI::EBlendOp::Subtract:
        return VK_BLEND_OP_SUBTRACT;
    case RHI::EBlendOp::ReverseSubtract:
        return VK_BLEND_OP_REVERSE_SUBTRACT;
    case RHI::EBlendOp::Min:
        return VK_BLEND_OP_MIN;
    case RHI::EBlendOp::Max:
        return VK_BLEND_OP_MAX;
    }
}

static void Convert(VkStencilOpState& dst, const CStencilOpState& src)
{
    dst.failOp = VkCast(src.FailOp);
    dst.passOp = VkCast(src.PassOp);
    dst.depthFailOp = VkCast(src.DepthFailOp);
    dst.compareOp = VkCast(src.CompareOp);
    dst.compareMask = src.CompareMask;
    dst.writeMask = src.WriteMask;
    dst.reference = 0; // Dynamic
}

static void Convert(VkPipelineColorBlendAttachmentState& dst, const CRenderTargetBlendDesc& src)
{
    dst.blendEnable = src.BlendEnable;
    dst.srcColorBlendFactor = VkCast(src.SrcBlend);
    dst.dstColorBlendFactor = VkCast(src.DestBlend);
    dst.colorBlendOp = VkCast(src.BlendOp);
    dst.srcAlphaBlendFactor = VkCast(src.SrcBlendAlpha);
    dst.dstAlphaBlendFactor = VkCast(src.DestBlendAlpha);
    dst.alphaBlendOp = VkCast(src.BlendOpAlpha);
    dst.colorWriteMask = static_cast<VkColorComponentFlags>(src.RenderTargetWriteMask);
}

CPipelineVk::CPipelineVk(CDeviceVk& p, const CPipelineDesc& desc)
    : Parent(p)
{
    EntryPoints.reserve(5);
    AddShaderModule(desc.VS, VK_SHADER_STAGE_VERTEX_BIT);
    AddShaderModule(desc.PS, VK_SHADER_STAGE_FRAGMENT_BIT);
    AddShaderModule(desc.GS, VK_SHADER_STAGE_GEOMETRY_BIT);
    AddShaderModule(desc.DS, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    AddShaderModule(desc.HS, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    // Catagorize each binding by set id
    for (const auto& pair : ResourceByName)
    {
        const auto& resource = pair.second;

        auto set = SetBindings.find(resource.Set);
        if (set != SetBindings.end())
        {
            set->second.push_back(resource);
        }
        else
        {
            SetBindings.emplace(resource.Set, std::vector<CPipelineResource> { resource });
        }
    }

    // Create descriptor set layout
    for (const auto& pair : SetBindings)
    {
        auto set = pair.first;
        CDescriptorSetLayoutVk* layout = nullptr;
        Parent.GetDescriptorSetLayoutCache()->CreateLayout(set, pair.second, &layout);
        SetLayouts.emplace(set, layout);
    }

    // Create pipeline layout
    std::vector<VkDescriptorSetLayout> setLayouts;
    for (auto it : SetLayouts)
        setLayouts.push_back(it.second->GetHandle());

    std::vector<VkPushConstantRange> pushConstantRanges;
    for (auto it : ResourceByName)
    {
        const auto& resource = it.second;
        if (resource.ResourceType == EPipelineResourceType::PushConstantBuffer)
        {
            VkPushConstantRange range = {};
            range.stageFlags = static_cast<VkShaderStageFlags>(resource.Stages);
            range.offset = resource.Offset;
            range.size = resource.Size;
            pushConstantRanges.push_back(range);
        }
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    pipelineLayoutInfo.pSetLayouts = setLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();
    VkResult result =
        vkCreatePipelineLayout(Parent.GetVkDevice(), &pipelineLayoutInfo, nullptr, &PipelineLayout);
    if (result != VK_SUCCESS)
        throw CRHIRuntimeError("Vulkan pipeline layout create failed");

    // Create a pipeline create info and fill in handles
    auto renderpass = std::static_pointer_cast<CRenderPassVk>(desc.RenderPass);
    VkGraphicsPipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    pipelineInfo.stageCount = static_cast<uint32_t>(StageInfos.size());
    pipelineInfo.pStages = StageInfos.data();
    pipelineInfo.layout = PipelineLayout;
    pipelineInfo.renderPass = renderpass->RenderPass;
    pipelineInfo.subpass = 0; // TODO

    // Translate vertex input states
    std::vector<VkVertexInputBindingDescription> bindingDesc;
    for (const auto& it : desc.VertexBindings)
    {
        bindingDesc.push_back(VkVertexInputBindingDescription());
        bindingDesc.back().binding = it.Binding;
        bindingDesc.back().stride = it.Stride;
        bindingDesc.back().inputRate =
            it.bIsPerInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
    }
    std::vector<VkVertexInputAttributeDescription> attribDesc;
    for (const auto& it : desc.VertexAttributes)
    {
        attribDesc.push_back(VkVertexInputAttributeDescription());
        attribDesc.back().binding = it.Binding;
        attribDesc.back().location = it.Location;
        attribDesc.back().format = static_cast<VkFormat>(it.Format);
        attribDesc.back().offset = it.Offset;
    }
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    };
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDesc.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDesc.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribDesc.size());
    vertexInputInfo.pVertexAttributeDescriptions = attribDesc.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;

    // Vertex assembly state, which is just topology
    VkPipelineInputAssemblyStateCreateInfo iaInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO
    };
    iaInfo.topology = VkCast(desc.PrimitiveTopology);
    iaInfo.primitiveRestartEnable = VK_TRUE;
    pipelineInfo.pInputAssemblyState = &iaInfo;

    // Tesselation state lol
    if (desc.HS && desc.DS)
    {
        VkPipelineTessellationStateCreateInfo tessInfo = {
            VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO
        };
        tessInfo.patchControlPoints = desc.PatchControlPoints;
        pipelineInfo.pTessellationState = &tessInfo;
    }

    // Viewport state. Although we use dynamic we still need to specify the number
    VkPipelineViewportStateCreateInfo viewportInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
    };
    viewportInfo.viewportCount = 1;
    viewportInfo.scissorCount = 1;
    pipelineInfo.pViewportState = &viewportInfo;

    // Rasterization state
    VkPipelineRasterizationStateCreateInfo rastInfo {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
    };
    if (desc.RasterizerState)
    {
        rastInfo.depthClampEnable = desc.RasterizerState->DepthClampEnable;
        rastInfo.rasterizerDiscardEnable = VK_FALSE;
        rastInfo.polygonMode = VkCast(desc.RasterizerState->PolygonMode);
        rastInfo.cullMode = VkCast(desc.RasterizerState->CullMode);
        rastInfo.frontFace = desc.RasterizerState->FrontFaceCCW ? VK_FRONT_FACE_COUNTER_CLOCKWISE
                                                                : VK_FRONT_FACE_CLOCKWISE;
        rastInfo.depthBiasEnable = desc.RasterizerState->DepthBiasEnable;
        rastInfo.depthBiasConstantFactor = desc.RasterizerState->DepthBiasConstantFactor;
        rastInfo.depthBiasClamp = desc.RasterizerState->DepthBiasClamp;
        rastInfo.depthBiasSlopeFactor = desc.RasterizerState->DepthBiasSlopeFactor;
        rastInfo.lineWidth = 1.0f;
        pipelineInfo.pRasterizationState = &rastInfo;
    }

    // Multisample state
    VkPipelineMultisampleStateCreateInfo msInfo {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO
    };
    if (desc.MultisampleState)
    {
        msInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        msInfo.sampleShadingEnable = VK_FALSE;
        msInfo.minSampleShading = 1.0f;
        msInfo.pSampleMask = nullptr;
        msInfo.alphaToCoverageEnable = VK_FALSE;
        msInfo.alphaToOneEnable = VK_FALSE;
        pipelineInfo.pMultisampleState = &msInfo;
        // TODO: implement multisampling
    }

    // Depth stencil state
    VkPipelineDepthStencilStateCreateInfo dsInfo {
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
    };
    if (desc.DepthStencilState)
    {
        dsInfo.depthTestEnable = desc.DepthStencilState->DepthTestEnable;
        dsInfo.depthWriteEnable = desc.DepthStencilState->DepthWriteEnable;
        dsInfo.depthCompareOp = VkCast(desc.DepthStencilState->DepthCompareOp);
        dsInfo.depthBoundsTestEnable = VK_FALSE;
        dsInfo.stencilTestEnable = desc.DepthStencilState->StencilTestEnable;
        Convert(dsInfo.front, desc.DepthStencilState->Front);
        Convert(dsInfo.back, desc.DepthStencilState->Back);
        dsInfo.minDepthBounds = 0.0f;
        dsInfo.maxDepthBounds = 1.0f;
        pipelineInfo.pDepthStencilState = &dsInfo;
    }

    // Blend state
    VkPipelineColorBlendStateCreateInfo blendInfo {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO
    };
    blendInfo.logicOpEnable = VK_FALSE;
    blendInfo.logicOp = VK_LOGIC_OP_CLEAR;
    blendInfo.attachmentCount = renderpass->SubpassColorAttachmentCount(pipelineInfo.subpass);
    std::vector<VkPipelineColorBlendAttachmentState> attachmentBlend;
    if (desc.BlendState && blendInfo.attachmentCount)
    {
        attachmentBlend.resize(blendInfo.attachmentCount);
        Convert(attachmentBlend[0], desc.BlendState->RenderTargets[0]);
        for (size_t i = 1; i < attachmentBlend.size(); i++)
            if (!desc.BlendState->IndependentBlendEnable)
                attachmentBlend[i] = attachmentBlend[0];
            else
                Convert(attachmentBlend[i], desc.BlendState->RenderTargets[i]);
        blendInfo.pAttachments = attachmentBlend.data();
        pipelineInfo.pColorBlendState = &blendInfo;

        if (blendInfo.attachmentCount > 8)
            throw CRHIException("My assumption was wrong after all");
    }

    // Dynamic states! To match d3d11 behavior
    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO
    };
    std::array<VkDynamicState, 4> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT,
                                                    VK_DYNAMIC_STATE_SCISSOR,
                                                    VK_DYNAMIC_STATE_BLEND_CONSTANTS,
                                                    VK_DYNAMIC_STATE_STENCIL_REFERENCE };
    dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateInfo.pDynamicStates = dynamicStates.data();
    pipelineInfo.pDynamicState = &dynamicStateInfo;

    result = vkCreateGraphicsPipelines(Parent.GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo,
                                       nullptr, &PipelineHandle);
    if (result != VK_SUCCESS)
        throw CRHIRuntimeError("Failed to create vulkan pipeline");
}

CPipelineVk::~CPipelineVk()
{
    if (PipelineHandle != VK_NULL_HANDLE)
        vkDestroyPipeline(Parent.GetVkDevice(), PipelineHandle, nullptr);

    if (PipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(Parent.GetVkDevice(), PipelineLayout, nullptr);

    for (auto it : SetLayouts)
        Parent.GetDescriptorSetLayoutCache()->DestroyLayout(it.second);
}

const std::unordered_map<uint32_t, std::vector<CPipelineResource>>
CPipelineVk::GetSetBindings() const
{
    return SetBindings;
}

CDescriptorSetLayoutVk* CPipelineVk::GetSetLayout(uint32_t set) const
{
    auto iter = SetLayouts.find(set);
    if (iter == SetLayouts.end())
        return nullptr;
    return iter->second;
}

void CPipelineVk::AddShaderModule(CShaderModule::Ref shaderModule, VkShaderStageFlagBits stage)
{
    if (!shaderModule)
        return;

    auto smImpl = std::static_pointer_cast<CShaderModuleVk>(shaderModule);

    EntryPoints.push_back(smImpl->GetEntryPoint());

    VkPipelineShaderStageCreateInfo stageInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
    };
    stageInfo.stage = stage;
    stageInfo.module = std::static_pointer_cast<CShaderModuleVk>(shaderModule)->GetVkModule();
    stageInfo.pName = EntryPoints.back().c_str();
    stageInfo.pSpecializationInfo = nullptr;

    StageInfos.push_back(stageInfo);

    // Grab all resources from each individual shader and put them into a big hash map
    for (const auto& resource : smImpl->GetShaderResources())
    {
        // Is name really the best choice for the key???
        auto key = std::string(resource.Name);
        if (resource.ResourceType == EPipelineResourceType::StageOutput
            || resource.ResourceType == EPipelineResourceType::StageInput)
            key = std::to_string(resource.Stages) + ":" + key;

        auto it = ResourceByName.find(key);
        if (it != ResourceByName.end())
            it->second.Stages |= resource.Stages;
        else
            ResourceByName.emplace(key, resource);
    }
}

}
