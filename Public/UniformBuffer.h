#pragma once
#include "Resources.h"
#include <Color.h>
#include <Matrix4.h>
#include <Vector3.h>
#include <Vector4.h>
#include <memory>
#include <string>

namespace RHI
{

// Import types and define type traits
using tc::Color;
using tc::Matrix4;
using tc::Vector2;
using tc::Vector3;
using tc::Vector4;

template <typename T> struct TUniformBufferMemberTraits;

template <> struct TUniformBufferMemberTraits<bool>
{
    static constexpr int Align = 4;
    static constexpr const char* HLSLKeyword = "bool";
};

template <> struct TUniformBufferMemberTraits<int>
{
    static constexpr int Align = 4;
    static constexpr const char* HLSLKeyword = "int";
};

template <> struct TUniformBufferMemberTraits<unsigned int>
{
    static constexpr int Align = 4;
    static constexpr const char* HLSLKeyword = "uint";
};

template <> struct TUniformBufferMemberTraits<float>
{
    static constexpr int Align = 4;
    static constexpr const char* HLSLKeyword = "float";
};

template <> struct TUniformBufferMemberTraits<double>
{
    static constexpr int Align = 4;
    static constexpr const char* HLSLKeyword = "double";
};

template <> struct TUniformBufferMemberTraits<Color>
{
    static constexpr int Align = 16;
    static constexpr const char* HLSLKeyword = "float4";
};

template <> struct TUniformBufferMemberTraits<Vector2>
{
    static constexpr int Align = 16;
    static constexpr const char* HLSLKeyword = "float2";
};

template <> struct TUniformBufferMemberTraits<Vector3>
{
    static constexpr int Align = 16;
    static constexpr const char* HLSLKeyword = "float3";
};

template <> struct TUniformBufferMemberTraits<Vector4>
{
    static constexpr int Align = 16;
    static constexpr const char* HLSLKeyword = "float4";
};

template <> struct TUniformBufferMemberTraits<Matrix4>
{
    static constexpr int Align = 16;
    static constexpr const char* HLSLKeyword = "float4x4";
};

// TODO: think of a better name
template <typename T> class TTypedUniformBufferRef
{
public:
    TTypedUniformBufferRef(CBuffer::Ref inputBuffer)
        : BufferPtr(inputBuffer)
    {
    }

    CBuffer::Ref GetBuffer() const { return BufferPtr; }

private:
    CBuffer::Ref BufferPtr;
};

// Uniform reflector
class CUniformMemberDecl
{
public:
    CUniformMemberDecl(std::string keyword, std::string name);

private:
    std::string HLSLKeyword;
    std::string Name;
};

#define BEGIN_UNIFORM_BUFFER(Name)                                                                 \
    class Name                                                                                     \
    {                                                                                              \
        struct _BeginTypeId                                                                        \
        {                                                                                          \
        };                                                                                         \
        static std::vector<RHI::CUniformMemberDecl> GetMembersSoFar(_BeginTypeId) { return {}; }   \
        typedef _BeginTypeId

#define MEMBER(Type, Name)                                                                         \
    _PrevTypeId##Name;                                                                             \
                                                                                                   \
public:                                                                                            \
    alignas(RHI::TUniformBufferMemberTraits<Type>::Align) Type Name;                               \
                                                                                                   \
private:                                                                                           \
    struct _TypeId##Name                                                                           \
    {                                                                                              \
    };                                                                                             \
    static std::vector<RHI::CUniformMemberDecl> GetMembersSoFar(_TypeId##Name)                     \
    {                                                                                              \
        auto result = GetMembersSoFar(_PrevTypeId##Name());                                        \
        result.emplace_back(RHI::TUniformBufferMemberTraits<Type>::HLSLKeyword, #Name);            \
        return result;                                                                             \
    }                                                                                              \
    typedef _TypeId##Name

#define END_UNIFORM_BUFFER()                                                                       \
    _FinalTypeId;                                                                                  \
    }                                                                                              \
    ;

} /* namespace RHI */
