#pragma once
#include "Resources.h"
#include "Sampler.h"
#include <CompileTimeHash.h>
#include <cassert>
#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace RHI
{

enum class EShaderStageFlags
{
    Vertex = 0x00000001,
    TessellationControl = 0x00000002,
    TessellationEvaluation = 0x00000004,
    Geometry = 0x00000008,
    Pixel = 0x00000010,
    Compute = 0x00000020,
    AllGraphics = 0x0000001f,
    All = 0x7fffffff
};

DEFINE_ENUM_CLASS_BITWISE_OPERATORS(EShaderStageFlags)

// Describes a single vertex buffer input slot
struct CVertexShaderInputDesc
{
    EFormat Format;
    std::string Name; // For debugging only, presumably
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
        CBuffer::Ref Buffer;
        uint32_t Offset;

        bool operator=(const CBoundAccessor& rhs) const
        {
            return Binding == rhs.Binding && Buffer == rhs.Buffer && Offset == rhs.Offset;
        }
    };

    std::vector<CBoundAccessor> BoundAccessors;

    void AddAccessor(uint32_t binding, CBuffer::Ref buffer, uint32_t offset)
    {
        BoundAccessors.push_back(CBoundAccessor { binding, buffer, offset });
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

// Arguments supplied for the shaders within a pipeline
struct CPipelineArguments
{
    using CArgType = std::variant<CBuffer::Ref, CImageView::Ref, CSampler::Ref>;
    std::map<uint32_t, CArgType> Arguments;

    template <typename T> void Add(uint32_t id, const T& arg)
    {
        // TODO warn if override
        assert(arg);
        Arguments.insert_or_assign(id, CArgType(arg));
    }
};

struct HLSLSrc
{
};
struct DXBCBlob
{
};
struct SPIRVBlob
{
};

//"ShaderModule" may not be the most appropriate name for this class, since it merely represents an
// uncompiled shader
enum class EShaderFormat
{
    HLSLFile,
    DXBC
};

class CShaderModule
{
public:
    typedef std::shared_ptr<CShaderModule> Ref;

    CShaderModule() = default;
    CShaderModule(const std::string& sourcePath, const std::string& target,
                  const std::string& entryPoint, HLSLSrc);
    CShaderModule(const void* data, size_t size, DXBCBlob);
    CShaderModule(const void* data, size_t size, SPIRVBlob);

    virtual ~CShaderModule() = default;

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
