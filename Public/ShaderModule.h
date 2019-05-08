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

#if TC_OS == TC_OS_LINUX
#undef Bool
#endif

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

//"ShaderModule" may not be the most appropriate name for this class, since it merely represents an
// uncompiled shader
enum class EShaderFormat
{
    HLSLFile,
    DXBC
};

// The following 3 definitions are used for spriv shader reflection
enum class EPipelineResourceType : uint8_t
{
    StageInput,
    StageOutput,
    SeparateSampler,
    CombinedImageSampler,
    SeparateImage,
    StorageImage,
    UniformTexelBuffer,
    StorageTexelBuffer,
    UniformBuffer,
    StorageBuffer,
    SubpassInput,
    PushConstantBuffer,
};

enum class EBaseType
{
    Bool,
    Char,
    Int,
    UInt,
    Half,
    Float,
    Double,
    Struct
};

const int MaxDescriptionSize = 256;

struct CPipelineResource
{
    EShaderStageFlags Stages;
    EPipelineResourceType ResourceType;
    EBaseType BaseType;
    uint32_t Access;
    uint32_t Set;
    uint32_t Binding;
    uint32_t Location;
    uint32_t InputAttachmentIndex;
    uint32_t VecSize;
    uint32_t Columns;
    uint32_t ArraySize;
    uint32_t Offset;
    uint32_t Size;
    char Name[MaxDescriptionSize];
};


class RHI_API CShaderModule : public tc::FNonCopyable
{
public:
    typedef std::shared_ptr<CShaderModule> Ref;

    CShaderModule() = default;
    CShaderModule(const std::string& sourcePath, const std::string& target,
                  const std::string& entryPoint, HLSLSrc);
    CShaderModule(const void* data, size_t size, DXBCBlob);

    virtual ~CShaderModule() = default;

    bool operator=(const CShaderModule& rhs) const;
    std::string GetShaderCacheKey() const;
    EShaderFormat GetShaderFormat() const
    {
        if (bIsDXBC)
            return EShaderFormat::DXBC;
        return EShaderFormat::HLSLFile;
    }

    virtual const std::vector<CPipelineResource>& GetShaderResources() const = 0;

private:
    bool bIsDXBC = false;

    std::string SourcePath;
    std::string Target;
    std::string EntryPoint;

    std::vector<char> DataBlob;
    std::string DataBlobHash;
};

} /* namespace RHI */
