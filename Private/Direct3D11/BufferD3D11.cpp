#include "BufferD3D11.h"
#include "RHIException.h"

namespace RHI
{

CBufferD3D11::CBufferD3D11(ID3D11Device* d3dDevice, uint32_t size, EBufferUsageFlags usage, const void* initialData)
    : CBufferBase(size, usage)
{
    //Determine bind flags
    UINT bindFlags = 0;
    if (Any(usage, EBufferUsageFlags::VertexBuffer))
        bindFlags |= D3D11_BIND_VERTEX_BUFFER;
    if (Any(usage, EBufferUsageFlags::IndexBuffer))
        bindFlags |= D3D11_BIND_INDEX_BUFFER;

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = size;
    bd.BindFlags = bindFlags;
    bd.CPUAccessFlags = 0;

    HRESULT hr;
    if (initialData)
    {
        D3D11_SUBRESOURCE_DATA InitData = {};
        InitData.pSysMem = initialData;
        hr = d3dDevice->CreateBuffer(&bd, &InitData, BufferPtr.GetAddressOf());
    }
    else
    {
        hr = d3dDevice->CreateBuffer(&bd, nullptr, BufferPtr.GetAddressOf());
    }

    if (FAILED(hr))
        throw CRHIRuntimeError("Could not create buffer");

    d3dDevice->GetImmediateContext(ImmediateContext.GetAddressOf());
}

void* CBufferD3D11::Map(size_t offset, size_t size)
{
    //TODO: write performance opt
    D3D11_MAPPED_SUBRESOURCE mapped;
    ImmediateContext->Map(BufferPtr.Get(), 0, D3D11_MAP_WRITE, 0, &mapped);
    return mapped.pData;
}

void CBufferD3D11::Unmap()
{
    ImmediateContext->Unmap(BufferPtr.Get(), 0);
}

} /* namespace RHI */
