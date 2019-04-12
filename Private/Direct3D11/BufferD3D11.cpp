#include "BufferD3D11.h"
#include "RHIException.h"

namespace RHI
{

CBufferD3D11::CBufferD3D11(ID3D11Device* d3dDevice, size_t size, EBufferUsageFlags usage,
                           const void* initialData)
    : CBufferBase(size, usage)
{
    // Determine bind flags
    UINT bindFlags = 0;
    D3dUsage = D3D11_USAGE_DEFAULT;
    UINT accessFlags = 0;
    if (Any(usage, EBufferUsageFlags::VertexBuffer))
        bindFlags |= D3D11_BIND_VERTEX_BUFFER;
    if (Any(usage, EBufferUsageFlags::IndexBuffer))
        bindFlags |= D3D11_BIND_INDEX_BUFFER;
    if (Any(usage, EBufferUsageFlags::ConstantBuffer))
    {
        bindFlags |= D3D11_BIND_CONSTANT_BUFFER;
        D3dUsage = D3D11_USAGE_DYNAMIC;
        accessFlags |= D3D11_CPU_ACCESS_WRITE;
    }

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3dUsage;
    bd.ByteWidth = static_cast<UINT>(size);
    bd.BindFlags = bindFlags;
    bd.CPUAccessFlags = accessFlags;

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
    // TODO: write performance opt
    D3D11_MAPPED_SUBRESOURCE mapped;
	D3D11_MAP mapMethod = D3D11_MAP_WRITE;
    if (D3dUsage == D3D11_USAGE_DYNAMIC)
        mapMethod = D3D11_MAP_WRITE_DISCARD;
    ImmediateContext->Map(BufferPtr.Get(), 0, mapMethod, 0, &mapped);
    return static_cast<uint8_t*>(mapped.pData) + offset;
}

void CBufferD3D11::Unmap() { ImmediateContext->Unmap(BufferPtr.Get(), 0); }

} /* namespace RHI */
