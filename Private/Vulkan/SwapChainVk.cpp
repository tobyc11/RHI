#include "SwapChainVk.h"
#include "DeviceVk.h"
#include "ImageViewVk.h"
#include "ImageVk.h"

namespace RHI
{

CPhysicalDeviceSwapChainCaps CSwapChainVk::GetDeviceSwapChainCaps(CDeviceVk& device,
                                                                  VkSurfaceKHR surface)
{
    CPhysicalDeviceSwapChainCaps caps;
    caps.AssociatedSurface = surface;
    caps.PhysicalDevice = device.GetVkPhysicalDevice();

    VkBool32 presentSupported;
    vkGetPhysicalDeviceSurfaceSupportKHR(caps.PhysicalDevice, device.GetQueueFamily(QT_GRAPHICS),
                                         surface, &presentSupported);
    if (presentSupported)
    {
        caps.PresentQueue = device.GetQueueFamily(QT_GRAPHICS);
    }
    else
    {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(caps.PhysicalDevice, &queueFamilyCount, nullptr);
        for (uint32_t i = 0; i < queueFamilyCount; i++)
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(caps.PhysicalDevice, i, surface,
                                                 &presentSupported);
            if (presentSupported)
            {
                caps.PresentQueue = i;
                break;
            }
        }
    }

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(caps.PhysicalDevice, surface, &caps.Capabilities);

    uint32_t surfaceFormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(caps.PhysicalDevice, surface, &surfaceFormatCount,
                                         nullptr);
    if (surfaceFormatCount != 0)
    {
        caps.SurfaceFormats.resize(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(caps.PhysicalDevice, surface, &surfaceFormatCount,
                                             caps.SurfaceFormats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(caps.PhysicalDevice, surface, &presentModeCount,
                                              nullptr);
    if (presentModeCount != 0)
    {
        caps.PresentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(caps.PhysicalDevice, surface, &presentModeCount,
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
        if (mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
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
    , PhysicalDevice(caps.PhysicalDevice)
{
    auto chosenFormat = SelectImageFormat(caps);

    CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    CreateInfo.pNext = nullptr;
    CreateInfo.flags = 0;
    CreateInfo.surface = caps.AssociatedSurface;
    CreateInfo.minImageCount = caps.Capabilities.minImageCount + 1;
    CreateInfo.imageFormat = chosenFormat.format;
    CreateInfo.imageColorSpace = chosenFormat.colorSpace;
    CreateInfo.imageExtent = SelectSurfaceExtent(caps);
    CreateInfo.imageArrayLayers = 1;
    CreateInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    CreateInfo.queueFamilyIndexCount = 0;
    CreateInfo.pQueueFamilyIndices = nullptr;
    CreateInfo.preTransform = caps.Capabilities.currentTransform;
    CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    CreateInfo.presentMode = SelectPresentMode(caps);
    CreateInfo.clipped = VK_TRUE;
    CreateInfo.oldSwapchain = VK_NULL_HANDLE;

    CreateSwapChainAndImages();
}

CSwapChainVk::~CSwapChainVk()
{
    ReleaseSwapChainAndImages();
    vkDestroySurfaceKHR(Parent.GetVkInstance(), CreateInfo.surface, nullptr);
}

void CSwapChainVk::Resize(uint32_t width, uint32_t height)
{
    vkDeviceWaitIdle(Parent.GetVkDevice());

    CreateInfo.imageExtent.width = width;
    CreateInfo.imageExtent.height = height;
    CreateInfo.oldSwapchain = SwapChainHandle;

    if (width == UINT32_MAX && height == UINT32_MAX)
    {
        VkSurfaceCapabilitiesKHR caps;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, CreateInfo.surface, &caps);
        CreateInfo.imageExtent = caps.currentExtent;
    }

    ReleaseSwapChainAndImages(true);
    CreateSwapChainAndImages();
    vkDestroySwapchainKHR(Parent.GetVkDevice(), CreateInfo.oldSwapchain, nullptr);
    CreateInfo.oldSwapchain = VK_NULL_HANDLE;
}

void CSwapChainVk::GetSize(uint32_t& width, uint32_t& height) const
{
    width = CreateInfo.imageExtent.width;
    height = CreateInfo.imageExtent.height;
}

CImage::Ref CSwapChainVk::GetImage()
{
    if (!ProxyImage)
        ProxyImage = std::make_shared<CSwapChainImageVk>(Parent, this->weak_from_this());
    return ProxyImage;
}

bool CSwapChainVk::AcquireNextImage()
{
    VkSemaphore imageAvailableSemaphore;
    VkSemaphoreCreateInfo semaphoreInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkResult result;
    result =
        vkCreateSemaphore(Parent.GetVkDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphore);
    if (result != VK_SUCCESS)
        throw CRHIRuntimeError("failed to create semaphores!");

    uint32_t imageIndex;
    result = vkAcquireNextImageKHR(Parent.GetVkDevice(), SwapChainHandle, UINT64_MAX,
                                   imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    if (result != VK_SUCCESS)
    {
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
            bNeedResize = true;
        return false;
    }

    AcquiredImages.push(std::make_pair(imageIndex, imageAvailableSemaphore));
    std::static_pointer_cast<CSwapChainImageVk>(ProxyImage)
        ->InitializeAccess(0, 0, VK_IMAGE_LAYOUT_UNDEFINED);
    return true;
}

void CSwapChainVk::Present(const CSwapChainPresentInfo& info)
{
    auto ctx = std::static_pointer_cast<CCommandContextVk>(Parent.GetImmediateContext());
    VkSemaphore renderJobDone = ctx->GetSignalSemaphore();
    ctx->Flush(false, true);

    VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderJobDone;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &SwapChainHandle;
    presentInfo.pImageIndices = &AcquiredImages.front().first;
    presentInfo.pResults = nullptr;
    vkQueuePresentKHR(Parent.GetVkQueue(QT_GRAPHICS), &presentInfo);

    vkDestroySemaphore(Parent.GetVkDevice(), AcquiredImages.front().second, nullptr);
    AcquiredImages.pop();
}

void CSwapChainVk::ReleaseSwapChainAndImages(bool dontDeleteSwapchain)
{
    for (auto iv : ImageViews)
        vkDestroyImageView(Parent.GetVkDevice(), iv, nullptr);
    ImageViews.clear();
    Images.clear();
    if (!dontDeleteSwapchain)
        vkDestroySwapchainKHR(Parent.GetVkDevice(), SwapChainHandle, nullptr);
}

void CSwapChainVk::CreateSwapChainAndImages()
{
    // Create swapchain object
    vkCreateSwapchainKHR(Parent.GetVkDevice(), &CreateInfo, nullptr, &SwapChainHandle);

    // Get the images
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(Parent.GetVkDevice(), SwapChainHandle, &imageCount, nullptr);
    Images.resize(imageCount);
    vkGetSwapchainImagesKHR(Parent.GetVkDevice(), SwapChainHandle, &imageCount, Images.data());

    // Create a view for each image
    VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewInfo.image = VK_NULL_HANDLE;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = CreateInfo.imageFormat;
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

}
