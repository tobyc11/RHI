#pragma once
#include "SwapChain.h"

namespace RHI
{

class IViewportClient;
class CRenderGraph;

//Represents a window, widget, offscreen buffer, etc.
class IViewport
{
public:
    IViewport(IViewportClient* client) : Client(client) {}
    virtual ~IViewport() = default;

    virtual float GetAspectRatio() const = 0;
    virtual float GetWidth() const = 0;
    virtual float GetHeight() const = 0;

    virtual tc::sp<CSwapChain> GetSwapChain() const = 0;
    virtual CRenderGraph* GetRenderGraph() = 0;

protected:
    IViewportClient* Client;
};

//Something that draws and possibly receives inputs from a viewport
class IViewportClient
{
public:
    //TODO: more like update
    virtual void Draw(IViewport* vp) = 0;
    
    //OnSizeChange, maybe?

    virtual bool OnMousePress(IViewport* vp, uint32_t buttons, int x, int y) = 0;
    virtual bool OnMouseRelease(IViewport* vp, uint32_t buttons, int x, int y) = 0;
    virtual bool OnMouseMove(IViewport* vp, int x, int y) = 0;
    virtual bool OnMouseWheel(IViewport* vp, int degrees) = 0;
};

} /* namespace Nome */
