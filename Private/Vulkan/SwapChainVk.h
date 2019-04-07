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
    VkSurfaceKHR AssociatedSurface;
    VkSurfaceCapabilitiesKHR Capabilities;
    std::vector<VkSurfaceFormatKHR> SurfaceFormats;
    std::vector<VkPresentModeKHR> PresentModes;
    uint32_t PresentQueue;
};

class CSwapChainVk : public CSwapChain
{
public:
    static CPhysicalDeviceSwapChainCaps GetDeviceSwapChainCaps(VkSurfaceKHR surface,
                                                               VkPhysicalDevice device);
    static VkSurfaceFormatKHR SelectImageFormat(const CPhysicalDeviceSwapChainCaps& caps);
    static VkPresentModeKHR SelectPresentMode(const CPhysicalDeviceSwapChainCaps& caps);
    static VkExtent2D SelectSurfaceExtent(const CPhysicalDeviceSwapChainCaps& caps);
	
    CSwapChainVk(CDeviceVk& device, const CPhysicalDeviceSwapChainCaps& caps);
    ~CSwapChainVk() override;

    void Resize(uint32_t width, uint32_t height) override;
    void GetSize(uint32_t& width, uint32_t& height) const override;

    CImage::Ref GetImage() override;
    void AcquireNextImage() override;
    void Present(const CSwapChainPresentInfo& info) override;

    // Internal api
    const VkSurfaceFormatKHR& GetChosenFormat() const { return ChosenFormat; }
    const std::vector<VkImageView>& GetVkImageViews() const { return ImageViews; }
    std::queue<std::pair<uint32_t, VkSemaphore>> AcquiredImages;

private:
    CDeviceVk& Parent;
    VkSurfaceKHR Surface;

    VkSurfaceFormatKHR ChosenFormat;
    VkPresentModeKHR ChosenPresentMode;
    VkExtent2D CurrentExtent;

    VkSwapchainKHR SwapChainHandle;
    std::vector<VkImage> Images;
    std::vector<VkImageView> ImageViews;
    CImage::Ref ProxyImage;
};

}
