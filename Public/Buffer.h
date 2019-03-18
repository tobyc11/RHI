#pragma once
#include "RHIChooseImpl.h"
#include <EnumClass.h>
#include <RefBase.h>
#include <cstdint>

namespace RHI
{

using tc::sp;

enum class EBufferUsageFlags
{
    VertexBuffer = 1,
    IndexBuffer = 2,
    ConstantBuffer = 4,
    Streaming = 8,
};

DEFINE_ENUM_CLASS_BITWISE_OPERATORS(EBufferUsageFlags);

template <typename TDerived>
class CBufferBase : public tc::TLightRefBase<CBufferBase<TDerived>>
{
protected:
    CBufferBase(uint32_t size, EBufferUsageFlags usage);

public:
    virtual ~CBufferBase() = default;

    void* Map(size_t offset, size_t size);
    void Unmap();

private:
    uint32_t Size;
    EBufferUsageFlags Usage;
};

using CBuffer = TChooseImpl<CBufferBase>::TConcreteBase;

struct CBufferView
{
    sp<CBuffer> Buffer;
    uint32_t Stride;
    uint32_t Offset;
};

} /* namespace RHI */
