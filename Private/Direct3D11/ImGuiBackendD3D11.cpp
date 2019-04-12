#ifdef RHI_HAS_IMGUI
#include "DeviceD3D11.h"
#include "RHIImGuiBackend.h"
#include "imgui_impl_dx11.h"

namespace RHI
{

void CRHIImGuiBackend::Init(CDevice::Ref device, CRenderPass::Ref renderPass)
{
    auto deviceImpl = std::static_pointer_cast<CDeviceD3D11>(device);
    ImGui_ImplDX11_Init(deviceImpl->D3dDevice.Get(), deviceImpl->ImmediateContext.Get());
}

void CRHIImGuiBackend::Shutdown() { ImGui_ImplDX11_Shutdown(); }

void CRHIImGuiBackend::NewFrame() { ImGui_ImplDX11_NewFrame(); }

void CRHIImGuiBackend::RenderDrawData(ImDrawData* draw_data, IRenderContext::Ref context)
{
    ImGui_ImplDX11_RenderDrawData(draw_data);
}

}
#endif
