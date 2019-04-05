#include "ShaderModuleVk.h"
#include "DeviceVk.h"
#include "SPIRVReflection.h"

namespace RHI
{

CShaderModuleVk::CShaderModuleVk(CDeviceVk& p, size_t size, const void* pCode)
    : Parent(p)
{
    SPIRVBlob.resize(size / sizeof(uint32_t));
    memcpy(SPIRVBlob.data(), pCode, size);

	spirv_cross::CompilerGLSL compiler(SPIRVBlob);
    Stage = SPIRVGetStage(compiler);
	EntryPoint = "main"; //TODO: big assumption

	SPIRVReflectResources(compiler, Stage, Resources);

    VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    createInfo.codeSize = size;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(pCode);

    vkCreateShaderModule(Parent.GetVkDevice(), &createInfo, nullptr, &ShaderModule);
}

CShaderModuleVk::~CShaderModuleVk()
{
    vkDestroyShaderModule(Parent.GetVkDevice(), ShaderModule, nullptr);
}

}
