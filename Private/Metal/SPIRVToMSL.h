#pragma once
#include "ShaderModule.h"
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace RHI
{

struct CMSLBindingRemap
{
    // Maps (set, binding) -> flat Metal index for each resource type
    std::map<std::pair<uint32_t, uint32_t>, uint32_t> BufferBindings;
    std::map<std::pair<uint32_t, uint32_t>, uint32_t> TextureBindings;
    std::map<std::pair<uint32_t, uint32_t>, uint32_t> SamplerBindings;
};

struct CMSLCompileResult
{
    std::string Source;
    CMSLBindingRemap Remap;
    std::vector<CPipelineResource> Resources;
};

CMSLCompileResult CompileSPIRVToMSL(const uint32_t* spirvCode, size_t wordCount);

} /* namespace RHI */
