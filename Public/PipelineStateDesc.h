#pragma once
#include <EnumClass.h>
#include <cstdint>
#include <vector>

namespace RHI
{

struct CViewportDesc
{
    float X;
    float Y;
    float Width;
    float Height;
    float MinDepth;
    float MaxDepth;

    CViewportDesc& SetX(float value) { X = value; return *this; }
    CViewportDesc& SetY(float value) { Y = value; return *this; }
    CViewportDesc& SetWidth(float value) { Width = value; return *this; }
    CViewportDesc& SetHeight(float value) { Height = value; return *this; }
    CViewportDesc& SetMinDepth(float value) { MinDepth = value; return *this; }
    CViewportDesc& SetMaxDepth(float value) { MaxDepth = value; return *this; }
};

struct CRectDesc
{
    int32_t X;
    int32_t Y;
    uint32_t Width;
    uint32_t Height;

    CRectDesc& SetX(int32_t value) { X = value; return *this; }
    CRectDesc& SetY(int32_t value) { Y = value; return *this; }
    CRectDesc& SetWidth(uint32_t value) { Width = value; return *this; }
    CRectDesc& SetHeight(uint32_t value) { Height = value; return *this; }
};

enum class EPolygonMode
{
    Fill,
    Wireframe
};

enum class ECullModeFlags
{
    None = 0,
    Front = 1,
    Back = 1 << 1
};

DEFINE_ENUM_CLASS_BITWISE_OPERATORS(ECullModeFlags)

struct CRasterizerDesc
{
    bool DepthClampEnable;
    bool RasterizerDiscardEnable;
    EPolygonMode PolygonMode;
    ECullModeFlags CullMode;
    bool FrontFaceCCW;
    bool DepthBiasEnable;
    float DepthBiasConstantFactor;
    float DepthBiasClamp;
    float DepthBiasSlopeFactor;
    float LineWidth;

    CRasterizerDesc& SetDepthClampEnable(bool value) { DepthClampEnable = value; return *this; }
    CRasterizerDesc& SetRasterizerDiscardEnable(bool value) { RasterizerDiscardEnable = value; return *this; }
    CRasterizerDesc& SetPolygonMode(EPolygonMode value) { PolygonMode = value; return *this; }
    CRasterizerDesc& SetCullMode(ECullModeFlags value) { CullMode = value; return *this; }
    CRasterizerDesc& SetFrontFaceCCW(bool value) { FrontFaceCCW = value; return *this; }
    CRasterizerDesc& SetDepthBiasEnable(bool value) { DepthBiasEnable = value; return *this; }
    CRasterizerDesc& SetDepthBiasConstantFactor(float value) { DepthBiasConstantFactor = value; return *this; }
    CRasterizerDesc& SetDepthBiasClamp(float value) { DepthBiasClamp = value; return *this; }
    CRasterizerDesc& SetDepthBiasSlopeFactor(float value) { DepthBiasSlopeFactor = value; return *this; }
    CRasterizerDesc& SetLineWidth(float value) { LineWidth = value; return *this; }

    bool operator==(const CRasterizerDesc& rhs) const
    {
        return DepthClampEnable == rhs.DepthClampEnable &&
            RasterizerDiscardEnable == rhs.RasterizerDiscardEnable &&
            PolygonMode == rhs.PolygonMode &&
            CullMode == rhs.CullMode &&
            FrontFaceCCW == rhs.FrontFaceCCW &&
            DepthBiasEnable == rhs.DepthBiasEnable &&
            DepthBiasConstantFactor == rhs.DepthBiasConstantFactor &&
            DepthBiasClamp == rhs.DepthBiasClamp &&
            DepthBiasSlopeFactor == rhs.DepthBiasSlopeFactor &&
            LineWidth == rhs.LineWidth;
    }

    friend std::size_t hash_value(const CRasterizerDesc& r)
    {
        std::size_t result = 0;
        result |= static_cast<std::size_t>(r.DepthClampEnable); result <<= 1;
        result |= static_cast<std::size_t>(r.RasterizerDiscardEnable); result <<= 1;
        result |= static_cast<std::size_t>(r.PolygonMode); result <<= 1;
        result |= static_cast<std::size_t>(r.CullMode); result <<= 1;
        result |= static_cast<std::size_t>(r.FrontFaceCCW); result <<= 1;
        result |= static_cast<std::size_t>(r.DepthBiasEnable); result <<= 1;
        result |= static_cast<std::size_t>(r.DepthBiasConstantFactor); result <<= 1;
        result |= static_cast<std::size_t>(r.DepthBiasClamp); result <<= 1;
        result |= static_cast<std::size_t>(r.DepthBiasSlopeFactor); result <<= 1;
        result |= static_cast<std::size_t>(r.LineWidth); result <<= 1;
        return result;
    }
};

//Multisampling omitted

enum class ECompareOp
{
    Never = 0,
    Less = 1,
    Equal = 2,
    LessEqual = 3,
    Greater = 4,
    NotEqual = 5,
    GreaterEqual = 6,
    Always = 7,
};

enum class EStencilOp
{
    Keep = 0,
    Zero = 1,
    Replace = 2,
    IncrementAndClamp = 3,
    DecrementAndClamp = 4,
    Invert = 5,
    IncrementAndWrap = 6,
    DecrementAndWrap = 7,
};

struct CStencilOpState
{
    EStencilOp FailOp;
    EStencilOp PassOp;
    EStencilOp DepthFailOp;
    ECompareOp CompareOp;
    uint32_t CompareMask;
    uint32_t WriteMask;
    uint32_t Reference;

    CStencilOpState& SetFailOp(EStencilOp value) { FailOp = value; return *this; }
    CStencilOpState& SetPassOp(EStencilOp value) { PassOp = value; return *this; }
    CStencilOpState& SetDepthFailOp(EStencilOp value) { DepthFailOp = value; return *this; }
    CStencilOpState& SetCompareOp(ECompareOp value) { CompareOp = value; return *this; }
    CStencilOpState& SetCompareMask(uint32_t value) { CompareMask = value; return *this; }
    CStencilOpState& SetWriteMask(uint32_t value) { WriteMask = value; return *this; }
    CStencilOpState& SetReference(uint32_t value) { Reference = value; return *this; }

    bool operator==(const CStencilOpState& rhs) const
    {
        return FailOp == rhs.FailOp &&
            PassOp == rhs.PassOp &&
            DepthFailOp == rhs.DepthFailOp &&
            CompareOp == rhs.CompareOp &&
            CompareMask == rhs.CompareMask &&
            WriteMask == rhs.WriteMask &&
            Reference == rhs.Reference;
    }

    friend std::size_t hash_value(const CStencilOpState& r)
    {
        std::size_t result = 0;
        result |= static_cast<std::size_t>(r.FailOp); result <<= 1;
        result |= static_cast<std::size_t>(r.PassOp); result <<= 1;
        result |= static_cast<std::size_t>(r.DepthFailOp); result <<= 1;
        result |= static_cast<std::size_t>(r.CompareOp); result <<= 1;
        result |= static_cast<std::size_t>(r.CompareMask); result <<= 1;
        result |= static_cast<std::size_t>(r.WriteMask); result <<= 1;
        result |= static_cast<std::size_t>(r.Reference); result <<= 1;
        return result;
    }
};

struct CDepthStencilDesc
{
    bool DepthTestEnable;
    bool DepthWriteEnable;
    ECompareOp DepthCompareOp;
    bool StencilTestEnable;
    CStencilOpState Front;
    CStencilOpState Back;
    bool DepthBoundsTestEnable;
    float MinDepthBounds;
    float MaxDepthBounds;

    CDepthStencilDesc& SetDepthTestEnable(bool value) { DepthTestEnable = value; return *this; }
    CDepthStencilDesc& SetDepthWriteEnable(bool value) { DepthWriteEnable = value; return *this; }
    CDepthStencilDesc& SetDepthCompareOp(ECompareOp value) { DepthCompareOp = value; return *this; }
    CDepthStencilDesc& SetStencilTestEnable(bool value) { StencilTestEnable = value; return *this; }
    CDepthStencilDesc& SetFront(CStencilOpState value) { Front = value; return *this; }
    CDepthStencilDesc& SetBack(CStencilOpState value) { Back = value; return *this; }
    CDepthStencilDesc& SetDepthBoundsTestEnable(bool value) { DepthBoundsTestEnable = value; return *this; }
    CDepthStencilDesc& SetMinDepthBounds(float value) { MinDepthBounds = value; return *this; }
    CDepthStencilDesc& SetMaxDepthBounds(float value) { MaxDepthBounds = value; return *this; }

    bool operator==(const CDepthStencilDesc& rhs) const
    {
        return DepthTestEnable == rhs.DepthTestEnable &&
            DepthWriteEnable == rhs.DepthWriteEnable &&
            DepthCompareOp == rhs.DepthCompareOp &&
            StencilTestEnable == rhs.StencilTestEnable &&
            Front == rhs.Front &&
            Back == rhs.Back &&
            DepthBoundsTestEnable == rhs.DepthBoundsTestEnable &&
            MinDepthBounds == rhs.MinDepthBounds &&
            MaxDepthBounds == rhs.MaxDepthBounds;
    }

    friend std::size_t hash_value(const CDepthStencilDesc& r)
    {
        std::size_t result = 0;
        result |= static_cast<std::size_t>(r.DepthTestEnable); result <<= 1;
        result |= static_cast<std::size_t>(r.DepthWriteEnable); result <<= 1;
        result |= static_cast<std::size_t>(r.DepthCompareOp); result <<= 1;
        result |= static_cast<std::size_t>(r.StencilTestEnable); result <<= 1;
        result |= static_cast<std::size_t>(r.DepthBoundsTestEnable); result <<= 1;
        result |= static_cast<std::size_t>(r.MinDepthBounds); result <<= 1;
        result |= static_cast<std::size_t>(r.MaxDepthBounds); result <<= 1;
        return result;
    }
};

enum class EBlendMode
{
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    DstColor,
    OneMinusDstColor,
    SrcAlphaSaturate,
    //TODO: Custom blend factor support
};

enum class EBlendOp
{
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max
};

enum class EColorComponentFlags : uint8_t
{
    None = 0,
    Red = 1,
    Green = 1 << 1,
    Blue = 1 << 2,
    Alpha = 1 << 3,
    All = Red | Green | Blue | Alpha
};

DEFINE_ENUM_CLASS_BITWISE_OPERATORS(EColorComponentFlags)

struct CRenderTargetBlendDesc
{
    bool BlendEnable;
    EBlendMode SrcBlend;
    EBlendMode DestBlend;
    EBlendOp BlendOp;
    EBlendMode SrcBlendAlpha;
    EBlendMode DestBlendAlpha;
    EBlendOp BlendOpAlpha;
    EColorComponentFlags RenderTargetWriteMask;

    CRenderTargetBlendDesc& SetBlendEnable(bool value) { BlendEnable = value; return *this; }
    CRenderTargetBlendDesc& SetSrcBlend(EBlendMode value) { SrcBlend = value; return *this; }
    CRenderTargetBlendDesc& SetDestBlend(EBlendMode value) { DestBlend = value; return *this; }
    CRenderTargetBlendDesc& SetBlendOp(EBlendOp value) { BlendOp = value; return *this; }
    CRenderTargetBlendDesc& SetSrcBlendAlpha(EBlendMode value) { SrcBlendAlpha = value; return *this; }
    CRenderTargetBlendDesc& SetDestBlendAlpha(EBlendMode value) { DestBlendAlpha = value; return *this; }
    CRenderTargetBlendDesc& SetBlendOpAlpha(EBlendOp value) { BlendOpAlpha = value; return *this; }
    CRenderTargetBlendDesc& SetRenderTargetWriteMask(EColorComponentFlags value) { RenderTargetWriteMask = value; return *this; }

    bool operator==(const CRenderTargetBlendDesc& rhs) const
    {
        return (BlendEnable == false && rhs.BlendEnable == false) ||
            BlendEnable == rhs.BlendEnable &&
            SrcBlend == rhs.SrcBlend &&
            DestBlend == rhs.DestBlend &&
            BlendOp == rhs.BlendOp &&
            SrcBlendAlpha == rhs.SrcBlendAlpha &&
            DestBlendAlpha == rhs.DestBlendAlpha &&
            BlendOpAlpha == rhs.BlendOpAlpha &&
            RenderTargetWriteMask == rhs.RenderTargetWriteMask;
    }

    friend std::size_t hash_value(const CRenderTargetBlendDesc& r)
    {
        std::size_t result = 0;
        result |= static_cast<std::size_t>(r.BlendEnable); result <<= 1;
        result |= static_cast<std::size_t>(r.SrcBlend); result <<= 1;
        result |= static_cast<std::size_t>(r.DestBlend); result <<= 1;
        result |= static_cast<std::size_t>(r.BlendOp); result <<= 1;
        result |= static_cast<std::size_t>(r.SrcBlendAlpha); result <<= 1;
        result |= static_cast<std::size_t>(r.DestBlendAlpha); result <<= 1;
        result |= static_cast<std::size_t>(r.BlendOpAlpha); result <<= 1;
        result |= static_cast<std::size_t>(r.RenderTargetWriteMask); result <<= 1;
        return result;
    }
};

struct CBlendDesc
{
    CRenderTargetBlendDesc RenderTargets[8];
    float BlendConstants[4];

    CBlendDesc& SetRenderTargets(size_t num, CRenderTargetBlendDesc desc)
    {
        RenderTargets[num] = desc;
    }

    CBlendDesc& SetRenderTargets(const std::vector<CRenderTargetBlendDesc>& value)
    {
        size_t bound = value.size() < 8 ? value.size() : 8;
        for (size_t i = 0; i < bound; i++)
        {
            RenderTargets[i] = value[i];
        }
    }

    CBlendDesc& SetBlendConstants(float r, float g, float b, float a)
    {
        BlendConstants[0] = r;
        BlendConstants[1] = g;
        BlendConstants[2] = b;
        BlendConstants[3] = a;
        return *this;
    }

    bool operator==(const CBlendDesc& rhs) const
    {
        return BlendConstants[0] == rhs.BlendConstants[0] &&
            BlendConstants[1] == rhs.BlendConstants[1] &&
            BlendConstants[2] == rhs.BlendConstants[2] &&
            BlendConstants[3] == rhs.BlendConstants[3] &&
            RenderTargets[0] == rhs.RenderTargets[0] &&
            RenderTargets[1] == rhs.RenderTargets[1] &&
            RenderTargets[2] == rhs.RenderTargets[2] &&
            RenderTargets[3] == rhs.RenderTargets[3] &&
            RenderTargets[4] == rhs.RenderTargets[4] &&
            RenderTargets[5] == rhs.RenderTargets[5] &&
            RenderTargets[6] == rhs.RenderTargets[6] &&
            RenderTargets[7] == rhs.RenderTargets[7];
    }

    friend std::size_t hash_value(const CBlendDesc& r)
    {
        std::size_t result = 0;
        result = hash_value(r.RenderTargets[0]);
        result |= hash_value(r.RenderTargets[1]);
        result |= static_cast<std::size_t>(r.BlendConstants[0]); result <<= 1;
        return result;
    }
};

} /* namespace RHI */
