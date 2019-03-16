#include "ShaderModule.h"
#include <picosha2.h>

namespace RHI
{

CShaderModule::CShaderModule(const std::string& sourcePath, const std::string& target, const std::string& entryPoint, HLSLSrc)
    : SourcePath(sourcePath), Target(target), EntryPoint(entryPoint)
{
}

bool CShaderModule::operator=(const CShaderModule & rhs) const
{
    return SourcePath == rhs.SourcePath && Target == rhs.Target && EntryPoint == rhs.EntryPoint;
}

std::string CShaderModule::GetShaderCacheKey() const
{
    picosha2::hash256_one_by_one hasher;
    hasher.process(SourcePath.begin(), SourcePath.end());
    hasher.process(Target.begin(), Target.end());
    hasher.process(EntryPoint.begin(), EntryPoint.end());
    hasher.finish();

    std::vector<unsigned char> hash(picosha2::k_digest_size);
    hasher.get_hash_bytes(hash.begin(), hash.end());

    return picosha2::get_hash_hex_string(hasher);
}

} /* namespace RHI */
