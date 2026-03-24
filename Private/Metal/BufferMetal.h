#pragma once
#include "MtlCommon.h"
#include "Resources.h"

namespace RHI
{

class CBufferMetal : public CBuffer
{
public:
    typedef std::shared_ptr<CBufferMetal> Ref;

    CBufferMetal(CDeviceMetal& parent, size_t size, EBufferUsageFlags usage,
                 const void* initialData);
    ~CBufferMetal() override;

    void* Map(size_t offset, size_t size);
    void Unmap();

    id GetMTLBuffer() const { return Buffer; }

private:
    CDeviceMetal& Parent;
    id Buffer;
    size_t MappedOffset = 0;
    size_t MappedSize = 0;
};

} /* namespace RHI */
