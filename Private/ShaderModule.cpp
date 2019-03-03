#include "ShaderModule.h"

namespace Nome::RHI
{

CShaderModule::CShaderModule(const std::string& sourcePath, const std::string& target, const std::string& entryPoint, HLSLSrc)
    : SourcePath(sourcePath), Target(target), EntryPoint(entryPoint)
{
}

} /* namespace Nome::RHI */
