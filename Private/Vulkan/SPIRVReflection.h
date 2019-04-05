#pragma once
#include "ShaderModuleVk.h"
#include <cstdint>
#include <spirv_glsl.hpp>
#include <vector>

namespace RHI
{

VkShaderStageFlagBits SPIRVGetStage(spirv_cross::CompilerGLSL& compiler);

bool SPIRVReflectResources(spirv_cross::CompilerGLSL& compiler, VkShaderStageFlagBits stage,
                           std::vector<CPipelineResource>& shaderResources);

}
