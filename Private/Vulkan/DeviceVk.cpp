#include "DeviceVk.h"

#include "ImageViewVk.h"
#include "ImageVk.h"
#include "PipelineVk.h"
#include "RenderPassVk.h"
#include "SamplerVk.h"
#include "ShaderModuleVk.h"
#include "SwapChainVk.h"
#include "VkHelpers.h"

#include <vector>
#include <cstring>

#if TC_OS == TC_OS_LINUX
#include <vulkan/vulkan_xcb.h>
#endif

namespace RHI
{

VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT flags,
                                                   VkDebugReportObjectTypeEXT objectType,
                                                   uint64_t object, size_t location,
                                                   int32_t messageCode, const char* pLayerPrefix,
                                                   const char* pMessage, void* pUserData)
{
    std::string type = "Vk Validation ";
    type += ((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) ? "Error: " : "Warning: ");
    std::cout << (type + std::string(pMessage) + "\n") << std::endl;
    return VK_FALSE;
}

static void initDebugCallback(VkInstance instance, VkDebugReportCallbackEXT* pCallback)
{
    VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
    callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    callbackCreateInfo.flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT
        | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    callbackCreateInfo.pfnCallback = &debugReportCallback;

    // Function to create a debug callback has to be dynamically queried from the instance...
    PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
    CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugReportCallbackEXT");

    CreateDebugReportCallback(instance, &callbackCreateInfo, nullptr, pCallback);
}

static VkInstance Instance;
static VkDebugReportCallbackEXT DebugRptCallback;

void InitRHIInstance()
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "TobyRHI";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "TobyRHI";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Enumerate supported extensions
    uint32_t extensionCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensionProps(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProps.data());
    std::cout << "Supported instance extensions on this machine" << std::endl;
    for (const auto& extProp : extensionProps)
        std::cout << extProp.extensionName << std::endl;

    // Enumerate supported layers
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> supportedLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, supportedLayers.data());
    std::cout << "Supported instance layers on this machine" << std::endl;
    for (const auto& prop : supportedLayers)
        std::cout << prop.layerName << std::endl;

    std::vector<const char*> requiredExtensions = { "VK_KHR_surface", "VK_EXT_debug_report",
#ifdef _WIN32
                                                    "VK_KHR_win32_surface"
#elif TC_OS == TC_OS_LINUX
                                                    "VK_KHR_xcb_surface"
#elif TC_OS == TC_OS_MAC_OS_X
                                                    "VK_MVK_macos_surface"
#endif
    };

    const std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };

#if defined(NDEBUG)
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &Instance);
    if (result != VK_SUCCESS)
        throw CRHIRuntimeError("Could not create vulkan instance");
    initDebugCallback(Instance, &DebugRptCallback);
}

void ShutdownRHIInstance()
{
    PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback = VK_NULL_HANDLE;
    DestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
        Instance, "vkDestroyDebugReportCallbackEXT");
    if (DestroyDebugReportCallback)
    {
        DestroyDebugReportCallback(Instance, DebugRptCallback, nullptr);
    }
}

static VkPhysicalDevice selectPhysicalDevice(const std::vector<VkPhysicalDevice>& devices,
                                             EDeviceCreateHints hints)
{
    VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
    uint64_t bestMemory = 0;

    for (const VkPhysicalDevice& device : devices)
    {
        VkPhysicalDeviceMemoryProperties memProps;
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceMemoryProperties(device, &memProps);
        vkGetPhysicalDeviceProperties(device, &properties);

        // Get local memory size from device
        uint64_t deviceMemory = 0;
        for (uint32_t i = 0; i < memProps.memoryHeapCount; i++)
        {
            if ((memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) > 0)
            {
                deviceMemory = memProps.memoryHeaps[i].size;
                break;
            }
        }

        // Save if best found so far
        if (hints == EDeviceCreateHints::NoHint
            && (bestDevice == VK_NULL_HANDLE || deviceMemory > bestMemory))
        {
            bestDevice = device;
            bestMemory = deviceMemory;
        }

        if (Any(hints, EDeviceCreateHints::Integrated)
            && properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        {
            bestDevice = device;
            bestMemory = deviceMemory;
        }

        if (Any(hints, EDeviceCreateHints::Discrete)
            && properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            bestDevice = device;
            bestMemory = deviceMemory;
        }
    }

    return bestDevice;
}

CDeviceVk::CDeviceVk(EDeviceCreateHints hints)
    : QueueFamilies { (uint32_t)-1, (uint32_t)-1, (uint32_t)-1 }
    , SubmissionTracker(*this)
{
    uint32_t physDeviceCount;
    std::vector<VkPhysicalDevice> physDevice;
    vkEnumeratePhysicalDevices(Instance, &physDeviceCount, nullptr);
    physDevice.resize(physDeviceCount);
    vkEnumeratePhysicalDevices(Instance, &physDeviceCount, physDevice.data());
    PhysicalDevice = selectPhysicalDevice(physDevice, hints);

    vkGetPhysicalDeviceProperties(PhysicalDevice, &Properties);
    printf("RHI Info: Device name = %s\n", Properties.deviceName);

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperites(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount,
                                             queueFamilyProperites.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        VkQueueFlags flags = queueFamilyProperites[i].queueFlags;
        if ((flags & VK_QUEUE_GRAPHICS_BIT) != 0)
            QueueFamilies[QT_GRAPHICS] = i;
        if ((flags & VK_QUEUE_COMPUTE_BIT) != 0)
            QueueFamilies[QT_COMPUTE] = i;
        if ((flags & VK_QUEUE_TRANSFER_BIT) != 0)
            QueueFamilies[QT_TRANSFER] = i;
    }

    // Enable all features
    VkPhysicalDeviceFeatures requiredFeatures;
    vkGetPhysicalDeviceFeatures(PhysicalDevice, &requiredFeatures);

    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    std::vector<float> queuePriorities;
    queuePriorities.reserve(1024);

    {
        VkDeviceQueueCreateInfo info = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
        uint32_t queueCount = queueFamilyProperites.at(QueueFamilies[QT_GRAPHICS]).queueCount;
        info.queueCount = queueCount;
        info.queueFamilyIndex = QueueFamilies[QT_GRAPHICS];
        for (uint32_t unused = 0; unused < queueCount; unused++)
            queuePriorities.push_back(1.0f);
        info.pQueuePriorities = &queuePriorities.back() - queueCount + 1;
        queueInfos.push_back(info);
    }
    if (QueueFamilies[QT_COMPUTE] != QueueFamilies[QT_GRAPHICS])
    {
        VkDeviceQueueCreateInfo info = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
        uint32_t queueCount = queueFamilyProperites.at(QueueFamilies[QT_COMPUTE]).queueCount;
        info.queueCount = queueCount;
        info.queueFamilyIndex = QueueFamilies[QT_COMPUTE];
        for (uint32_t unused = 0; unused < queueCount; unused++)
            queuePriorities.push_back(1.0f);
        info.pQueuePriorities = &queuePriorities.back() - queueCount + 1;
        queueInfos.push_back(info);
    }
    if (QueueFamilies[QT_TRANSFER] != QueueFamilies[QT_COMPUTE]
        && QueueFamilies[QT_TRANSFER] != QueueFamilies[QT_GRAPHICS])
    {
        VkDeviceQueueCreateInfo info = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
        uint32_t queueCount = queueFamilyProperites.at(QueueFamilies[QT_TRANSFER]).queueCount;
        info.queueCount = queueCount;
        info.queueFamilyIndex = QueueFamilies[QT_TRANSFER];
        for (uint32_t unused = 0; unused < queueCount; unused++)
            queuePriorities.push_back(1.0f);
        info.pQueuePriorities = &queuePriorities.back() - queueCount + 1;
        queueInfos.push_back(info);
    }

    std::vector<const char*> extensionNames = { "VK_KHR_swapchain" };

    // Logical Device
    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = (uint32_t)queueInfos.size();
    deviceInfo.pQueueCreateInfos = queueInfos.data();
    deviceInfo.enabledExtensionCount = (uint32_t)extensionNames.size();
    deviceInfo.ppEnabledExtensionNames = extensionNames.data();
    deviceInfo.pEnabledFeatures = &requiredFeatures;

    vkCreateDevice(PhysicalDevice, &deviceInfo, nullptr, &Device);

    for (uint32_t type = 0; type < NUM_QUEUE_TYPES; type++)
    {
        uint32_t queueCount = queueFamilyProperites.at(QueueFamilies[type]).queueCount;
        Queues[type].resize(queueCount);
        for (uint32_t i = 0; i < queueCount; i++)
        {
            vkGetDeviceQueue(Device, QueueFamilies[type], i, &Queues[type][i]);
        }
    }

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = PhysicalDevice;
    allocatorInfo.device = Device;

    vmaCreateAllocator(&allocatorInfo, &Allocator);

    VkPipelineCacheCreateInfo pipelineCacheInfo = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
    vkCreatePipelineCache(Device, &pipelineCacheInfo, nullptr, &PipelineCache);

    DescriptorSetLayoutCache = std::make_unique<CDescriptorSetLayoutCacheVk>(*this);
    HugeConstantBuffer = std::make_unique<CPersistentMappedRingBuffer>(
        *this, 33554432, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT); // 32M

    SubmissionTracker.Init();
    ImmediateContext =
        std::make_shared<CCommandContextVk>(*this, QT_GRAPHICS, ECommandContextKind::Immediate);
}

CDeviceVk::~CDeviceVk()
{
    ImmediateContext.reset();
    SubmissionTracker.Shutdown();
    HugeConstantBuffer.reset();
    DescriptorSetLayoutCache.reset();
    vkDestroyPipelineCache(Device, PipelineCache, nullptr);
    vmaDestroyAllocator(Allocator);
    vkDestroyDevice(Device, nullptr);
}

CImage::Ref CDeviceVk::InternalCreateImage(VkImageType type, EFormat format, EImageUsageFlags usage,
                                           uint32_t width, uint32_t height, uint32_t depth,
                                           uint32_t mipLevels, uint32_t arrayLayers,
                                           uint32_t sampleCount, const void* initialData)
{
    VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.imageType = type;
    imageInfo.format = static_cast<VkFormat>(format);
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = depth;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = arrayLayers;
    imageInfo.samples = static_cast<VkSampleCountFlagBits>(sampleCount);
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // BELOW
    imageInfo.usage = 0; // BELOW
    if (QueueFamilies[QT_TRANSFER] != QueueFamilies[QT_GRAPHICS])
    {
        imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        imageInfo.queueFamilyIndexCount = 3;
        imageInfo.pQueueFamilyIndices = QueueFamilies;
    }
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Allocate memory using the Vulkan Memory Allocator (unless memFlags has the NO_ALLOCATION bit
    // set).
    VmaAllocation allocation = VK_NULL_HANDLE;
    VkImage handle = VK_NULL_HANDLE;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY; // BELOW
    allocCreateInfo.flags = 0;

    // Determine memory flags based on usage
    EResourceState defaultState = EResourceState::General; // BELOW
    if (Any(usage, EImageUsageFlags::Sampled))
    {
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        defaultState = EResourceState::ShaderResource;
    }
    if (Any(usage, EImageUsageFlags::RenderTarget))
    {
        imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        defaultState = EResourceState::RenderTarget;
    }
    if (Any(usage, EImageUsageFlags::DepthStencil))
    {
        imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        defaultState = EResourceState::DepthWrite;
    }
    if (Any(usage, EImageUsageFlags::Staging))
    {
        imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        defaultState = EResourceState::CopySource;
    }

    VkResult result;
    result = vmaCreateImage(Allocator, &imageInfo, &allocCreateInfo, &handle, &allocation, nullptr);
    if (result != VK_SUCCESS)
        throw CRHIRuntimeError("Could not create image");

    // Create an Image class instance from handle.
    auto image =
        std::make_shared<CMemoryImageVk>(*this, handle, allocation, imageInfo, usage, defaultState);

    auto ctx = MakeTransientContext(QT_TRANSFER);
    auto cmdBuffer = ctx->GetBuffer();
    if (initialData)
    {
        ctx->TransitionImage(*image, EResourceState::CopyDest);

        // Prepare a staging buffer
        VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size = GetUncompressedImageFormatSize(imageInfo.format)
            * static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(depth);
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VmaAllocation stagingAlloc = VK_NULL_HANDLE;

        vmaCreateBuffer(Allocator, &bufferInfo, &allocInfo, &stagingBuffer, &stagingAlloc, nullptr);

        void* mappedData;
        vmaMapMemory(Allocator, stagingAlloc, &mappedData);
        memcpy(mappedData, initialData, bufferInfo.size);
        vmaUnmapMemory(Allocator, stagingAlloc);

        // Synchronously copy the content
        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { width, height, depth };
        vkCmdCopyBufferToImage(cmdBuffer, stagingBuffer, handle,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        PendingDeletionBuffers.emplace_back(stagingBuffer, stagingAlloc);
    }
    ctx->TransitionImage(*image, defaultState);
    ctx->Flush(true);
    return std::move(image);
}

CBuffer::Ref CDeviceVk::CreateBuffer(size_t size, EBufferUsageFlags usage, const void* initialData)
{
    return std::make_shared<CBufferVk>(*this, size, usage, initialData);
}

CImage::Ref CDeviceVk::CreateImage1D(EFormat format, EImageUsageFlags usage, uint32_t width,
                                     uint32_t mipLevels, uint32_t arrayLayers, uint32_t sampleCount,
                                     const void* initialData)
{
    return InternalCreateImage(VK_IMAGE_TYPE_1D, format, usage, width, 1, 1, mipLevels, arrayLayers,
                               sampleCount, initialData);
}

CImage::Ref CDeviceVk::CreateImage2D(EFormat format, EImageUsageFlags usage, uint32_t width,
                                     uint32_t height, uint32_t mipLevels, uint32_t arrayLayers,
                                     uint32_t sampleCount, const void* initialData)
{
    return InternalCreateImage(VK_IMAGE_TYPE_2D, format, usage, width, height, 1, mipLevels,
                               arrayLayers, sampleCount, initialData);
}

CImage::Ref CDeviceVk::CreateImage3D(EFormat format, EImageUsageFlags usage, uint32_t width,
                                     uint32_t height, uint32_t depth, uint32_t mipLevels,
                                     uint32_t arrayLayers, uint32_t sampleCount,
                                     const void* initialData)
{
    return InternalCreateImage(VK_IMAGE_TYPE_3D, format, usage, width, height, depth, mipLevels,
                               arrayLayers, sampleCount, initialData);
}

CImageView::Ref CDeviceVk::CreateImageView(const CImageViewDesc& desc, CImage::Ref image)
{
    return std::make_shared<CImageViewVk>(*this, desc, std::static_pointer_cast<CImageVk>(image));
}

CShaderModule::Ref CDeviceVk::CreateShaderModule(size_t size, const void* pCode)
{
    return std::make_shared<CShaderModuleVk>(*this, size, pCode);
}

CRenderPass::Ref CDeviceVk::CreateRenderPass(const CRenderPassDesc& desc)
{
    return std::make_shared<CRenderPassVk>(*this, desc);
}

CPipeline::Ref CDeviceVk::CreatePipeline(const CPipelineDesc& desc)
{
    return std::make_shared<CPipelineVk>(*this, desc);
}

CSampler::Ref CDeviceVk::CreateSampler(const CSamplerDesc& desc)
{
    return std::make_shared<CSamplerVk>(*this, desc);
}

IRenderContext::Ref CDeviceVk::GetImmediateContext() { return ImmediateContext; }

IRenderContext::Ref CDeviceVk::CreateDeferredContext() { throw "unimplemented"; }

CSwapChain::Ref CDeviceVk::CreateSwapChain(const CPresentationSurfaceDesc& info, EFormat format)
{
    VkSurfaceKHR surface;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    if (info.Type == EPresentationSurfaceDescType::Win32)
    {

        PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR =
            (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(Instance, "vkCreateWin32SurfaceKHR");
        VkWin32SurfaceCreateInfoKHR createInfo = {
            VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR
        };
        VkResult result;

        if (!vkCreateWin32SurfaceKHR)
        {
            throw CRHIException("vkCreateWin32SurfaceKHR is not enabled in the Vulkan instance.");
        }
        createInfo.hinstance = info.Win32.Instance;
        createInfo.hwnd = info.Win32.Window;
        result = vkCreateWin32SurfaceKHR(Instance, &createInfo, nullptr, &surface);
        if (result != VK_SUCCESS)
        {
            throw CRHIRuntimeError("vkCreateWin32SurfaceKHR failed");
        }
    }
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
    if (info.Type == EPresentationSurfaceDescType::MacOS)
    {
        VkMacOSSurfaceCreateInfoMVK createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.pView = info.MacOS.View;
        VkResult result;
        result = vkCreateMacOSSurfaceMVK(Instance, &createInfo, nullptr, &surface);
        if (result != VK_SUCCESS)
            throw CRHIRuntimeError("vkCreateMacOSSurfaceMVK failed");
    }
#endif
#ifdef VK_USE_PLATFORM_LINUX_KHR
    if (info.Type == EPresentationSurfaceDescType::Linux)
    {
        VkXcbSurfaceCreateInfoKHR surfaceInfo = {};
        surfaceInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        surfaceInfo.connection = info.Linux.xconn;
        surfaceInfo.window = info.Linux.window;

        VkResult res = vkCreateXcbSurfaceKHR(Instance, &surfaceInfo, nullptr, &surface);
        if (res != VK_SUCCESS)
            throw CRHIRuntimeError("vkCreateXcbSurfaceKHR failed");
    }
#endif
    else { throw CRHIException("CreateSwapChain received invalid presentation surface desc"); }

    auto swapchainCaps = CSwapChainVk::GetDeviceSwapChainCaps(surface, PhysicalDevice);
    if (!swapchainCaps.bIsSuitable)
        throw CRHIRuntimeError("Device is not suitable for presentation");
    auto swapchain = std::make_shared<CSwapChainVk>(*this, swapchainCaps);
    return std::move(swapchain);
}

VkInstance CDeviceVk::GetVkInstance() const { return Instance; }

CCommandContextVk::Ref CDeviceVk::MakeTransientContext(EQueueType qt)
{
    CCommandContextVk::Ref context =
        std::make_shared<CCommandContextVk>(*this, qt, ECommandContextKind::Transient);
    return context;
}

} /* namespace RHI */
