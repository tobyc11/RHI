#pragma once
#include "RenderPass.h"
#include "DeviceD3D11.h"
#include "CommandListD3D11.h"
#include <unordered_map>

namespace RHI
{

class CDrawPass::Impl
{
public:
    Impl(CDrawPass& owner);

    void BeginRecording();
    void Record(CPipelineStates states, const CDrawTemplate& drawTemplate);
    void FinishRecording();
    void Submit();

private:
    CDrawPass& Owner;
    CDeviceD3D11* DeviceImpl;
    std::unique_ptr<CCommandListD3D11> CommandList;
};

class CClearPass::Impl
{
public:
    Impl(CClearPass& owner);

    void Submit();

private:
    CClearPass& Owner;
    CDeviceD3D11* DeviceImpl;
};

} /* namespace RHI */
