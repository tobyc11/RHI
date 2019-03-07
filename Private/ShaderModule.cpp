#include "ShaderModule.h"

namespace Nome::RHI
{

CShaderModule::CShaderModule(const std::string& sourcePath, const std::string& target, const std::string& entryPoint, HLSLSrc)
    : SourcePath(sourcePath), Target(target), EntryPoint(entryPoint)
{
}

bool CShaderModule::operator=(const CShaderModule & rhs) const
{
    return SourcePath == rhs.SourcePath && Target == rhs.Target && EntryPoint == rhs.EntryPoint;
}

size_t CShaderModule::GetShaderCacheKey() const
{
    std::hash<std::string> hasher;
    return hasher(SourcePath) ^ hasher(Target) ^ hasher(EntryPoint);
}

} /* namespace Nome::RHI */
