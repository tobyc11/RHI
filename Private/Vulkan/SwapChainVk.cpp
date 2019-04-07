#include "SwapChainVk.h"
#include "DeviceVk.h"
#include "ImageViewVk.h"
#include "ImageVk.h"

namespace RHI
{

CPhysicalDeviceSwapChainCaps CSwapChainVk::GetDeviceSwapChainCaps(VkSurfaceKHR surface,
                                                                  VkPhysicalDevice device)
{
    CPhysicalDeviceSwapChainCaps caps;
    caps.AssociatedSurface = surface;

    VkBool32 presentSupported;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupported);
        if (presentSupported)
        {
            caps.PresentQueue = i;
            break;
        }
    }

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &caps.Capabilities);

    uint32_t surfaceFormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCount, nullptr);
    if (surfaceFormatCount != 0)
    {
        caps.SurfaceFormats.resize(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCount,
                                             caps.SurfaceFormats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        caps.PresentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
                                                  caps.PresentModes.data());
    }

    caps.bIsSuitable = !caps.SurfaceFormats.empty() && !caps.PresentModes.empty();
    return caps;
}

VkSurfaceFormatKHR CSwapChainVk::SelectImageFormat(const CPhysicalDeviceSwapChainCaps& caps)
{
    // If the list contains only one entry with undefined format
    // it means that there are no preferred surface formats and any can be chosen
    if ((caps.SurfaceFormats.size() == 1) && (caps.SurfaceFormats[0].format == VK_FORMAT_UNDEFINED))
    {
        return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
    }

    // Check if list contains most widely used R8 G8 B8 A8 format
    // with nonlinear color space
    for (const VkSurfaceFormatKHR& surfaceFmt : caps.SurfaceFormats)
    {
        if (surfaceFmt.format == VK_FORMAT_R8G8B8A8_UNORM)
        {
            return surfaceFmt;
        }
    }

    // Return the first format from the list
    return caps.SurfaceFormats[0];
}

VkPresentModeKHR CSwapChainVk::SelectPresentMode(const CPhysicalDeviceSwapChainCaps& caps)
{
    // FIFO present mode is always available
    // MAILBOX is the lowest latency V-Sync enabled mode (something like triple-buffering) so use it
    // if available
    for (VkPresentModeKHR mode : caps.PresentModes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return mode;
        }
    }
    for (VkPresentModeKHR mode : caps.PresentModes)
    {
        if (mode == VK_PRESENT_MODE_FIFO_KHR)
        {
            return mode;
        }
    }
    printf("%s", "FIFO present mode is not supported by the swap chain!");
    return static_cast<VkPresentModeKHR>(-1);
}

VkExtent2D CSwapChainVk::SelectSurfaceExtent(const CPhysicalDeviceSwapChainCaps& caps)
{
    // Special value of surface extent is width == height == -1
    // If this is so we define the size by ourselves but it must fit within defined confines
    if (caps.Capabilities.currentExtent.width == -1)
    {
        VkExtent2D result = { 640, 480 };
        if (result.width < caps.Capabilities.minImageExtent.width)
        {
            result.width = caps.Capabilities.minImageExtent.width;
        }
        if (result.height < caps.Capabilities.minImageExtent.height)
        {
            result.height = caps.Capabilities.minImageExtent.height;
        }
        if (result.width > caps.Capabilities.maxImageExtent.width)
        {
            result.width = caps.Capabilities.maxImageExtent.width;
        }
        if (result.height > caps.Capabilities.maxImageExtent.height)
        {
            result.height = caps.Capabilities.maxImageExtent.height;
        }
        return result;
    }

    // Most of the cases we define size of the swap_chain images equal to current window's size
    return caps.Capabilities.currentExtent;
}

CSwapChainVk::CSwapChainVk(CDeviceVk& device, const CPhysicalDeviceSwapChainCaps& caps)
    : Parent(device)
    , Surface(caps.AssociatedSurface)
{
    ChosenFormat = SelectImageFormat(caps);

    VkSwapchainCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.surface = Surface;
    createInfo.minImageCount = caps.Capabilities.minImageCount + 1;
    createInfo.imageFormat = ChosenFormat.format;
    createInfo.imageColorSpace = ChosenFormat.colorSpace;
    createInfo.imageExtent = CurrentExtent = SelectSurfaceExtent(caps);
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
    createInfo.preTransform = caps.Capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = ChosenPresentMode = SelectPresentMode(caps);
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    vkCreateSwapchainKHR(Parent.GetVkDevice(), &createInfo, nullptr, &SwapChainHandle);

    // Get the images
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(Parent.GetVkDevice(), SwapChainHandle, &imageCount, nullptr);
    Images.resize(imageCount);
    vkGetSwapchainImagesKHR(Parent.GetVkDevice(), SwapChainHandle, &imageCount, Images.data());

    // Create a view for each image
    VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewInfo.image = VK_NULL_HANDLE;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = ChosenFormat.format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;
    for (uint32_t i = 0; i < imageCount; i++)
    {
        VkImageView view;
        viewInfo.image = Images[i];
        vkCreateImageView(Parent.GetVkDevice(), &viewInfo, nullptr, &view);
        ImageViews.push_back(view);
    }
}

CSwapChainVk::~CSwapChainVk()
{
    for (auto iv : ImageViews)
        vkDestroyImageView(Parent.GetVkDevice(), iv, nullptr);
    vkDestroySwapchainKHR(Parent.GetVkDevice(), SwapChainHandle, nullptr);
    vkDestroySurfaceKHR(Parent.GetVkInstance(), Surface, nullptr);
}

void CSwapChainVk::Resize(uint32_t width, uint32_t height) { throw "unimplemented"; }

void CSwapChainVk::GetSize(uint32_t& width, uint32_t& height) const
{
    width = CurrentExtent.width;
    height = CurrentExtent.height;
}

CImage::Ref CSwapChainVk::GetImage()
{
    if (!ProxyImage)
        ProxyImage = std::make_shared<CImageVk>(Parent, this->weak_from_this());
    return ProxyImage;
}

void CSwapChainVk::AcquireNextImage()
{
    VkSemaphore imageAvailableSemaphore;
    VkSemaphoreCreateInfo semaphoreInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    if (vkCreateSemaphore(Parent.GetVkDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphore)
        != VK_SUCCESS)
    {
        throw CRHIRuntimeError("failed to create semaphores!");
    }

    uint32_t imageIndex;
    vkAcquireNextImageKHR(Parent.GetVkDevice(), SwapChainHandle, UINT64_MAX,
                          imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    AcquiredImages.push(std::make_pair(imageIndex, imageAvailableSemaphore));
}

void CSwapChainVk::Present(const CSwapChainPresentInfo& info)
{
    auto ctx = Parent.GetImmediateContext();
    ctx->Flush(true);

    VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    presentInfo.waitSemaphoreCount = 0;
    presentInfo.pWaitSemaphores = nullptr;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &SwapChainHandle;
    presentInfo.pImageIndices = &AcquiredImages.front().first;
    presentInfo.pResults = nullptr;
    vkQueuePresentKHR(Parent.GetVkQueue(GraphicsQueue), &presentInfo);

    vkDestroySemaphore(Parent.GetVkDevice(), AcquiredImages.front().second, nullptr);
    AcquiredImages.pop();
    vkDeviceWaitIdle(Parent.GetVkDevice());
}

}
