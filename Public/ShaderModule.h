#pragma once
#include "Format.h"
#include "Buffer.h"
#include "ImageView.h"
#include "Sampler.h"
#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace Nome::RHI
{

using tc::sp;

//Describes a single vertex buffer input slot
struct CVertexShaderInputDesc
{
    EFormat Format;
    uint32_t Location;
    std::string Name; //For debugging only, presumably
};

struct CVertexShaderInputSignature
{
    std::vector<CVertexShaderInputDesc> InputDescs;
};

struct CVertexShaderInputBinding
{
    struct CAccessor
    {
        sp<CBuffer> Buffer;
        uint32_t Offset;
        uint32_t Stride;
    };

    std::map<uint32_t, CAccessor> LocationToAccessor;

    void AddAccessor(uint32_t location, sp<CBuffer> buffer, uint32_t offset, uint32_t stride)
    {
        LocationToAccessor.insert(std::make_pair(location, CAccessor{ buffer, offset, stride }));
    }
};

struct CPixelShaderOutputDesc
{
    EFormat Format;
    uint32_t Location;
    std::string Name;
};

//Arguments supplied for the shaders within a pipeline
struct CPipelineArguments
{
    using CArgType = std::variant<sp<CBuffer>, sp<CImageView>, sp<CSampler>>;
    std::map<uint32_t, CArgType> Arguments;

    template <typename T>
    void Add(uint32_t id, const T&& arg)
    {
        Arguments.insert_or_assign(std::make_pair(id, CArgType(arg)));
    }
};

struct HLSLSrc{};

class CShaderModule : public tc::CVirtualLightRefBase
{
public:
    CShaderModule() = default;
    CShaderModule(const std::string& sourcePath, const std::string& target, const std::string& entryPoint, HLSLSrc);

    bool operator=(const CShaderModule& rhs) const;
    size_t GetShaderCacheKey() const;

private:
    friend class CShaderD3D11;

    std::string SourcePath;
    std::string Target;
    std::string EntryPoint;
};

} /* namespace Nome::RHI */
