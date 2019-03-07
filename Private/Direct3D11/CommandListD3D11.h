#pragma once
#include "DeviceD3D11.h"
#include "DrawTemplate.h"
#include <VectorPool.h>

namespace Nome::RHI
{

class CDrawCallCacheEntry
{
public:
    ComPtr<ID3D11RasterizerState> RastState;
    ComPtr<ID3D11DepthStencilState> DepthStencilState;
    ComPtr<ID3D11BlendState> BlendState;
};

class CCommandListD3D11
{
public:
    CCommandListD3D11(CDeviceD3D11* parent);

    using CDrawCallCacheEntryRef = tc::TVectorPool<CDrawCallCacheEntry>::VectorPoolReference;
    CDrawCallCacheEntryRef CacheDrawCall(const CDrawTemplate& drawTemplate);
    void Draw(CDrawCallCacheEntryRef cachedDraw);

private:
    CDeviceD3D11* Parent;
    ComPtr<ID3D11DeviceContext> DeferredCtx;
    tc::TVectorPool<CDrawCallCacheEntry> DrawCallCache;
};

} /* namespace Nome::RHI */
