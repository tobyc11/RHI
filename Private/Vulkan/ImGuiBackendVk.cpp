#ifdef RHI_HAS_IMGUI

#include "CommandContextVk.h"
#include "DeviceVk.h"
#include "RHIImGuiBackend.h"
#include "RenderPassVk.h"
#include "imgui_impl_vulkan.h"

namespace RHI
{

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    printf("VkResult %d\n", err);
    if (err < 0)
        abort();
}

static CDeviceVk::Ref DeviceImpl;
static VkPipelineCache PipelineCache = VK_NULL_HANDLE;
static VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
static bool bFontsUploaded = false;

void CRHIImGuiBackend::Init(CDevice::Ref device, CRenderPass::Ref renderPass)
{
    DeviceImpl = std::static_pointer_cast<CDeviceVk>(device);

    VkPipelineCacheCreateInfo pipelineCacheInfo = {};
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vkCreatePipelineCache(DeviceImpl->GetVkDevice(), &pipelineCacheInfo, nullptr, &PipelineCache);

    // Create Descriptor Pool
    {
        VkDescriptorPoolSize poolSizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                                             { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                                             { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                                             { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                                             { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                                             { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                                             { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                                             { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                                             { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                                             { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                                             { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };
        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
        poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
        poolInfo.pPoolSizes = poolSizes;
        VkResult err =
            vkCreateDescriptorPool(DeviceImpl->GetVkDevice(), &poolInfo, nullptr, &DescriptorPool);
        check_vk_result(err);
    }

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = DeviceImpl->GetVkInstance();
    init_info.PhysicalDevice = DeviceImpl->GetVkPhysicalDevice();
    init_info.Device = DeviceImpl->GetVkDevice();
    init_info.QueueFamily = DeviceImpl->GetQueueFamily(QT_GRAPHICS);
    init_info.Queue = DeviceImpl->GetVkQueue(QT_GRAPHICS);
    init_info.PipelineCache = PipelineCache;
    init_info.DescriptorPool = DescriptorPool;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = check_vk_result;

    ImGui_ImplVulkan_Init(&init_info,
                          std::static_pointer_cast<CRenderPassVk>(renderPass)->GetHandle());
}

void CRHIImGuiBackend::Shutdown()
{
    ImGui_ImplVulkan_Shutdown();
    vkDestroyPipelineCache(DeviceImpl->GetVkDevice(), PipelineCache, nullptr);
    vkDestroyDescriptorPool(DeviceImpl->GetVkDevice(), DescriptorPool, nullptr);
}

void CRHIImGuiBackend::NewFrame()
{
    if (!bFontsUploaded)
    {
        auto ctx = DeviceImpl->MakeTransientContext(QT_GRAPHICS);

        ImGui_ImplVulkan_CreateFontsTexture(ctx->GetBuffer());

        ctx->Flush(true); // TODO: might cause hitching
        ImGui_ImplVulkan_InvalidateFontUploadObjects();

        bFontsUploaded = true;
    }

    ImGui_ImplVulkan_NewFrame();
}

void CRHIImGuiBackend::RenderDrawData(ImDrawData* draw_data, IRenderContext::Ref context)
{
    auto contextImpl = std::static_pointer_cast<CCommandContextVk>(context);
    assert(contextImpl->IsInRenderPass());
    auto cmdBuffer = contextImpl->GetBuffer();
    ImGui_ImplVulkan_RenderDrawData(draw_data, cmdBuffer);
}

}

#endif
