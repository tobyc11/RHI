#pragma once
#include "Format.h"
#include "Buffer.h"
#include "ImageView.h"
#include "Sampler.h"
#include <CompileTimeHash.h>
#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace RHI
{

using tc::sp;

//Describes a single vertex buffer input slot
struct CVertexShaderInputDesc
{
    EFormat Format;
    std::string Name; //For debugging only, presumably
};

struct CVertexShaderInputSignature
{
    //       Location
    std::map<uint32_t, CVertexShaderInputDesc> InputDescs;
};

struct CVertexShaderInputBinding
{
    struct CBoundAccessor
    {
        uint32_t Location;
        sp<CBuffer> Buffer;
        uint32_t Stride;
        uint32_t Offset;

        bool operator=(const CBoundAccessor& rhs) const
        {
            return Location == rhs.Location && Buffer == rhs.Buffer && Offset == rhs.Offset && Stride == rhs.Stride;
        }
    };

    struct CLocationComparator
    {
        bool operator()(const CBoundAccessor& lhs, const CBoundAccessor& rhs) const
        {
            return lhs.Location < rhs.Location;
        }
    };

    struct CBufferAndStrideComparator
    {
        bool operator()(const CBoundAccessor& lhs, const CBoundAccessor& rhs) const
        {
            if (lhs.Buffer == rhs.Buffer)
                return lhs.Stride < rhs.Stride;
            return lhs.Buffer < rhs.Buffer;
        }
    };

    std::vector<CBoundAccessor> BoundAccessors;

    void AddAccessor(uint32_t location, sp<CBuffer> buffer, uint32_t offset, uint32_t stride)
    {
        BoundAccessors.push_back(CBoundAccessor{ location, buffer, offset, stride });
    }

    void AddAccessor(uint32_t location, const CBufferAccessor& a)
    {
        BoundAccessors.push_back(CBoundAccessor{ location, a.Buffer, a.Offset, a.Stride });
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
    void Add(uint32_t id, const T& arg)
    {
        //TODO warn if override
        Arguments.insert_or_assign(id, CArgType(arg));
    }
};

struct HLSLSrc {};
struct DXBCBlob {};

//"ShaderModule" may not be the most appropriate name for this class, since it merely represents an uncompiled shader
enum class EShaderFormat
{
    HLSLFile,
    DXBC
};

class CShaderModule : public tc::CVirtualLightRefBase
{
public:
    CShaderModule() = default;
    CShaderModule(const std::string& sourcePath, const std::string& target, const std::string& entryPoint, HLSLSrc);
    CShaderModule(const void* data, size_t size, DXBCBlob);

    bool operator=(const CShaderModule& rhs) const;
    std::string GetShaderCacheKey() const;
    EShaderFormat GetShaderFormat() const
    {
        if (bIsDXBC)
            return EShaderFormat::DXBC;
        return EShaderFormat::HLSLFile;
    }

private:
    friend class CShaderD3D11;

    bool bIsDXBC = false;

    std::string SourcePath;
    std::string Target;
    std::string EntryPoint;

    std::vector<char> DataBlob;
    std::string DataBlobHash;
};

} /* namespace RHI */
