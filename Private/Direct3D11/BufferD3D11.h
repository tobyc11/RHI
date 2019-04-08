#pragma once
#include "D3D11Platform.h"
#include "Resources.h"

namespace RHI
{

class CBufferD3D11 : public CBuffer
{
public:
    typedef std::shared_ptr<CBufferD3D11> Ref;

    CBufferD3D11(ID3D11Device* d3dDevice, size_t size, EBufferUsageFlags usage,
                 const void* initialData = nullptr);

    void* Map(size_t offset, size_t size);
    void Unmap();

    ID3D11Buffer* GetD3D11Buffer() const { return BufferPtr.Get(); }

private:
    // Grab the immediate context from the belonging device, so that we can do mapping
    ComPtr<ID3D11DeviceContext> ImmediateContext;
    ComPtr<ID3D11Buffer> BufferPtr;
    D3D11_USAGE D3dUsage;
};

} /* namespace RHI */
