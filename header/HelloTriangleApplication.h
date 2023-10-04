#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <algorithm> // Necessary for std::clamp
#include <iostream>
#include <optional>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <format>
#include <vector>
#include <set>

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily; // graphics / drawing
    std::optional<uint32_t> presentFamily; // presentation(window system integration)
    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

class HelloTriangleApplication
{
public:
    HelloTriangleApplication();
    ~HelloTriangleApplication();
    void run();
private:
    GLFWwindow*         window  {};

    VkInstance          instance {};
    // This debug messenger is create with a valid instance and destroyed before the instance is destroyed
    VkDebugUtilsMessengerEXT debugMessenger{};
    VkSurfaceKHR             surface{};

    VkPhysicalDevice    physicalDevice = VK_NULL_HANDLE;
    VkDevice            device {}; // logical device

    VkQueue             graphicsQueue {};
    VkQueue             presentQueue {};

    VkSwapchainKHR       swapChain{};
    std::vector<VkImage> swapChainImages{};
    VkFormat             swapChainImageFormat{};
    VkExtent2D           swapChainExtent{};


    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanUp();

    std::vector<const char*> getRequiredExtensions();
    void checkRequiredExtensionsSupport(std::vector<const char*>& glfwExtensions);
    void checkValidationLayerSupport();

    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    // Custom debug message callback function
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
};

