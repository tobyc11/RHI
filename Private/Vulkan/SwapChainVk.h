#pragma once
#include "PresentationSurfaceDesc.h"
#include "SwapChain.h"
#include "VkCommon.h"
#include <queue>
#include <utility>
#include <vector>

namespace RHI
{

struct CPhysicalDeviceSwapChainCaps
{
    bool bIsSuitable;
    VkPhysicalDevice PhysicalDevice;
    VkSurfaceKHR AssociatedSurface;
    VkSurfaceCapabilitiesKHR Capabilities;
    std::vector<VkSurfaceFormatKHR> SurfaceFormats;
    std::vector<VkPresentModeKHR> PresentModes;
    uint32_t PresentQueue;
};

class CSwapChainVk : public CSwapChain
{
public:
    typedef std::shared_ptr<CSwapChainVk> Ref;
    typedef std::weak_ptr<CSwapChainVk> WeakRef;

    static CPhysicalDeviceSwapChainCaps GetDeviceSwapChainCaps(CDeviceVk& device,
                                                               VkSurfaceKHR surface);
    static VkSurfaceFormatKHR SelectImageFormat(const CPhysicalDeviceSwapChainCaps& caps);
    static VkPresentModeKHR SelectPresentMode(const CPhysicalDeviceSwapChainCaps& caps);
    static VkExtent2D SelectSurfaceExtent(const CPhysicalDeviceSwapChainCaps& caps);

    CSwapChainVk(CDeviceVk& device, const CPhysicalDeviceSwapChainCaps& caps);
    ~CSwapChainVk() override;

    void Resize(uint32_t width, uint32_t height) override;
    void GetSize(uint32_t& width, uint32_t& height) const override;

    CImage::Ref GetImage() override;
    bool AcquireNextImage() override;
    void Present(const CSwapChainPresentInfo& info) override;

    // Internal api
    const VkFormat& GetChosenFormat() const { return CreateInfo.imageFormat; }
    const std::vector<VkImage>& GetVkImages() const { return Images; }
    const std::vector<VkImageView>& GetVkImageViews() const { return ImageViews; }
    std::queue<std::pair<uint32_t, VkSemaphore>> AcquiredImages;

private:
    void ReleaseSwapChainAndImages(bool dontDeleteSwapchain = false);
    void CreateSwapChainAndImages();

    CDeviceVk& Parent;
    VkPhysicalDevice PhysicalDevice;
    VkSwapchainCreateInfoKHR CreateInfo;
    bool bNeedResize = false;

    VkSwapchainKHR SwapChainHandle;
    std::vector<VkImage> Images;
    std::vector<VkImageView> ImageViews;
    CImage::Ref ProxyImage;
};

}
