#include "ShaderModule.h"
#include <picosha2.h>

namespace RHI
{

CShaderModule::CShaderModule(const std::string& sourcePath, const std::string& target,
                             const std::string& entryPoint, HLSLSrc)
    : bIsDXBC(false)
    , SourcePath(sourcePath)
    , Target(target)
    , EntryPoint(entryPoint)
{
}

CShaderModule::CShaderModule(const void* data, size_t size, DXBCBlob)
    : bIsDXBC(true)
{
    DataBlob.resize(size);
    memcpy(DataBlob.data(), data, size);

    std::vector<unsigned char> hash(picosha2::k_digest_size);
    picosha2::hash256(DataBlob.begin(), DataBlob.end(), hash.begin(), hash.end());

    DataBlobHash = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
}

bool CShaderModule::operator=(const CShaderModule& rhs) const
{
    if (bIsDXBC)
        return DataBlobHash == rhs.DataBlobHash;
    else
        return SourcePath == rhs.SourcePath && Target == rhs.Target && EntryPoint == rhs.EntryPoint;
}

std::string CShaderModule::GetShaderCacheKey() const
{
    if (bIsDXBC)
        return DataBlobHash;

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
