#pragma once
#include "Device.h"
#include "RenderContext.h"
#include <imgui.h>

namespace RHI
{

class CRHIImGuiBackend
{
public:
    // Initialize the ImGui renderer backend
    static void Init(CDevice::Ref device, CRenderPass::Ref renderPass);
    // Shutdown
    static void Shutdown();
    // Start a new frame, called before platform and imgui NewFrame()
    static void NewFrame();
    // Must be called within a render pass (compatible with the one passed into Init)
    static void RenderDrawData(ImDrawData* draw_data, IRenderContext& context);
};

} /* namespace RHI */
