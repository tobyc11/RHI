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

struct CVertexInputs
{
    struct CBoundAccessor
    {
        uint32_t Binding;
        sp<CBuffer> Buffer;
        uint32_t Offset;

        bool operator=(const CBoundAccessor& rhs) const
        {
            return Binding == rhs.Binding && Buffer == rhs.Buffer && Offset == rhs.Offset;
        }
    };

    std::vector<CBoundAccessor> BoundAccessors;

    void AddAccessor(uint32_t binding, sp<CBuffer> buffer, uint32_t offset)
    {
        BoundAccessors.push_back(CBoundAccessor{ binding, buffer, offset });
    }

    void AddAccessor(uint32_t binding, const CBufferView& a)
    {
        BoundAccessors.push_back(CBoundAccessor{ binding, a.Buffer, a.Offset });
    }
};

struct CVertexInputAttributeDesc
{
    uint32_t Location = 0;
    EFormat Format = EFormat::UNDEFINED;
    uint32_t Offset = 0;
    uint32_t Binding = 0;
};

struct CVertexInputBindingDesc
{
    uint32_t Binding = 0;
    uint32_t Stride = 0;
    bool bIsPerInstance = false;
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
