#ifdef RHI_HAS_IMGUI

#include "CommandContextMetal.h"
#include "DeviceMetal.h"
#include "RHIImGuiBackend.h"
#include "RenderPassMetal.h"

#include <imgui_impl_metal.h>

namespace RHI
{

static CDeviceMetal::Ref DeviceImpl;
static bool bFontsUploaded = false;

void CRHIImGuiBackend::Init(CDevice::Ref device, CRenderPass::Ref renderPass)
{
    DeviceImpl = std::static_pointer_cast<CDeviceMetal>(device);
    ImGui_ImplMetal_Init(DeviceImpl->GetMTLDevice());
}

void CRHIImGuiBackend::Shutdown()
{
    ImGui_ImplMetal_Shutdown();
    DeviceImpl.reset();
}

void CRHIImGuiBackend::NewFrame()
{
    if (!bFontsUploaded)
        bFontsUploaded = true;

    MTLRenderPassDescriptor* rpd = [MTLRenderPassDescriptor renderPassDescriptor];
    ImGui_ImplMetal_NewFrame(rpd);
}

void CRHIImGuiBackend::RenderDrawData(ImDrawData* draw_data, IRenderContext& context)
{
    auto& metalCtx = static_cast<CCommandContextMetal&>(context);
    id<MTLCommandBuffer> dummyCmdBuf = [DeviceImpl->GetDefaultQueue() commandBuffer];
    ImGui_ImplMetal_RenderDrawData(draw_data, dummyCmdBuf, metalCtx.GetRenderEncoder());
}

}

#endif
