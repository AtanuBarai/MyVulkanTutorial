#include "HelloTriangleApplication.h"
#include "DebugUtilsMessengerAndOtherHelper.h"

HelloTriangleApplication::HelloTriangleApplication()
{
    window = nullptr;
}

HelloTriangleApplication::~HelloTriangleApplication()
{
}

void HelloTriangleApplication::run()
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanUp();
}

void HelloTriangleApplication::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan: Hello Triangle!", nullptr, nullptr);
}

void HelloTriangleApplication::initVulkan()
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
}

void HelloTriangleApplication::mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

void HelloTriangleApplication::cleanUp()
{
    for (auto imageView : swapChainImageViews)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);
    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}

std::vector<const char*> HelloTriangleApplication::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }    
    return extensions;
}

void HelloTriangleApplication::checkRequiredExtensionsSupport(std::vector<const char*>& requiredExtensions)
{
    uint32_t supportedExtensionCount = 0;
    uint32_t requiredExtensionsCount = static_cast<uint32_t>(requiredExtensions.size());

    vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);
    print("\nExtension Support Check\n{} extensions supported\n", supportedExtensionCount);

    std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, supportedExtensions.data());

    auto isThisExtensionSupported = [&](const char* name) {
        for (auto& a : supportedExtensions)
        {
            if (strcmp(a.extensionName, name) == 0)
            {
                return true;
            }
        }
        return false;
    };

    for (auto requiredExtn: requiredExtensions)
    {
        print("Required extrension {} ", requiredExtn);
        if (!isThisExtensionSupported(requiredExtn))
        {
            print("not supported!\n");
            throw std::runtime_error("Some required extension/s not suported!\n");
        }
        print("supported\n");
    }
}

void HelloTriangleApplication::checkValidationLayerSupport()
{
    print("Validation Layer Check\n");

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    // Check if required validation layers are supported
    for (const auto & layerName : validationLayers)
    {
        print("Requested validation layer {} ", layerName);

        bool layerFound = false;
        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            print("not supported!\n");
            throw std::runtime_error("Some requested validation layer/s not suported!\n");
        }
        print("supported\n");
    }
}

bool HelloTriangleApplication::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    // Another way to check if extension is available
    for (const auto& extension : availableExtensions)
    {
        if (requiredExtensions.erase(extension.extensionName))
            print("Required device extension {} supported\n", extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool HelloTriangleApplication::isDeviceSuitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties  deviceProperties {};
    VkPhysicalDeviceFeatures    deviceFeatures {};

    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    //if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    //    return false;
    print("Device : {}, Api version supported {}\n", deviceProperties.deviceName, deviceProperties.apiVersion);

    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequete { false };
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequete = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    else
    {
        print("Required device extensions not supported\n");
        throw std::runtime_error("Required device extensions not supported\n");
    }

    // return false/0 if geometryShader and desired queue family is not supported
    return indices.isComplete() && deviceFeatures.geometryShader && extensionsSupported && swapChainAdequete;
}

QueueFamilyIndices HelloTriangleApplication::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0, i = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
        {
            print("Queue Family {} supports Graphics and Compute\n", i);
            indices.graphicsFamily = i;
        }

        // We can also select same queue family for both graphics/drawing and presentation(window system integration).
        // Then we need to move this two lines in the above 'if' statement
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport)
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
            break;
        i++;
    }

    return indices;
}

SwapChainSupportDetails HelloTriangleApplication::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details {};

    // query surface capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    // query the supported surface formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    // query supported presentation modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR HelloTriangleApplication::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availFmt : availableFormats)
    {
        if (availFmt.format == VK_FORMAT_B8G8R8A8_SRGB && availFmt.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
        {
            print("VK_FORMAT_B8G8R8A8_SRGB format and VK_COLORSPACE_SRGB_NONLINEAR_KHR colorspace supported\n");
            return availFmt;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR HelloTriangleApplication::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& presentMode : availablePresentModes)
    {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            print("VK_PRESENT_MODE_MAILBOX_KHR supported!\n");
            return presentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

/*
* The swap extent is the resolution of the swap chain images and it's almost always exactly equal to the resolution 
* of the window that we're drawing to in pixels. 
*/
VkExtent2D HelloTriangleApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width{}, height{};
        glfwGetFramebufferSize(window, &width, &height);
        
        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return actualExtent;
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplication::debugCallback(
                                        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                        void* pUserData)
{
    if(messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        print("Validation layer callback message: {} ",pCallbackData->pMessage);
    //std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}


void HelloTriangleApplication::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    // following is commented out cause default initializer sets it to nullptr
    // createInfo.pUserData = nullptr; // Optional, used to pass data to the "debugCallback"
}

/*
* The instance is the connection between your application and the Vulkan library and
* creating it involves specifying some details about your application to the driver.
*/
void HelloTriangleApplication::createInstance()
{
    if (enableValidationLayers) 
    {
        checkValidationLayerSupport();
    }
    VkApplicationInfo appInfo {};
    appInfo.sType               = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName    = "Hello Triangle";
    appInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName         = "No Engine";
    appInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion          = VK_API_VERSION_1_0;

    auto extensions = getRequiredExtensions();
    checkRequiredExtensionsSupport(extensions);

    VkInstanceCreateInfo createInfo {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo         = &appInfo;
    createInfo.enabledExtensionCount    = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames  = extensions.data();
    createInfo.enabledLayerCount        = 0;

    //VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {};
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        //populateDebugMessengerCreateInfo(debugCreateInfo);
        //createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    // VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance\n");
    }
    else
    {
        print("Instance created\n");
    }
}

void HelloTriangleApplication::setupDebugMessenger()
{
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);


    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to set up debug messenger!");
    }
    else
    {
        print("Debug messenger set up\n");
    }
}

void HelloTriangleApplication::createSurface()
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
    else
    {
        print("Surface created\n");
    }
}

void HelloTriangleApplication::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }
    
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device))
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void HelloTriangleApplication::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value() };

    if (indices.graphicsFamily.value() != indices.presentFamily.value())
    {
        print("Different queue family for graphics and presentation\n");
        uniqueQueueFamilies.insert(indices.presentFamily.value());
    }

    float queuePriority = 1.0f;
    for (const auto& queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    // device queue
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos    = queueCreateInfos.data();

    // device features
    createInfo.pEnabledFeatures = &deviceFeatures;

    // device extensions
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    // device specific validation layers for legacy vulkan support
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }
    // create logical device with the selected physical device and device info
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }
    else
    {
        print("Logical device created\n");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void HelloTriangleApplication::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE; // TODO: Check later
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }
    else
    {
        print("Swap Chain Created\n");
    }
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void HelloTriangleApplication::createImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }
        else
        {
            print("Image view created for swapchain {}\n", i);
        }
    }

}
