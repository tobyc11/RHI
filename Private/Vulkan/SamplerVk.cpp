#include "SamplerVk.h"
#include "DeviceVk.h"
#include "VkHelpers.h"

namespace RHI
{

CSamplerVk::CSamplerVk(CDeviceVk& p, const CSamplerDesc& desc)
    : Parent(p)
{
    VkSamplerCreateInfo samplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    samplerInfo.magFilter = VkCast(desc.MagFilter);
    samplerInfo.minFilter = VkCast(desc.MinFilter);
    samplerInfo.mipmapMode = VkCast(desc.MipmapMode);
    samplerInfo.addressModeU = VkCast(desc.AddressModeU);
    samplerInfo.addressModeV = VkCast(desc.AddressModeV);
    samplerInfo.addressModeW = VkCast(desc.AddressModeW);
    samplerInfo.mipLodBias = desc.MipLodBias;
    samplerInfo.anisotropyEnable = desc.AnisotropyEnable;
    samplerInfo.maxAnisotropy = desc.MaxAnisotropy;
    samplerInfo.compareEnable = desc.CompareEnable;
    samplerInfo.compareOp = VkCast(desc.CompareOp);
    samplerInfo.minLod = desc.MinLod;
    samplerInfo.maxLod = desc.MaxLod;
    if (desc.BorderColor[3] == 0.0f)
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    else if (desc.BorderColor[0] < 0.5f)
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    else
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

	vkCreateSampler(Parent.GetVkDevice(), &samplerInfo, nullptr, &Sampler);
}

CSamplerVk::~CSamplerVk() { vkDestroySampler(Parent.GetVkDevice(), Sampler, nullptr); }

} /* namespace RHI */
