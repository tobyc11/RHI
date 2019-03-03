#pragma once
#include "Buffer.h"
#include "D3D11Platform.h"

namespace Nome::RHI
{

class CBufferD3D11 : public CBufferBase<CBufferD3D11>
{
public:
    CBufferD3D11(ID3D11Device* d3dDevice, uint32_t size, EBufferUsageFlags usage, void* initialData = nullptr);

    void* Map(size_t offset, size_t size);
    void Unmap();

    ID3D11Buffer* GetD3D11Buffer() const
    {
        return BufferPtr.Get();
    }

private:
    //Grab the immediate context from the belonging device, so that we can do mapping
    ComPtr<ID3D11DeviceContext> ImmediateContext;
    ComPtr<ID3D11Buffer> BufferPtr;
};

} /* namespace Nome::RHI */
