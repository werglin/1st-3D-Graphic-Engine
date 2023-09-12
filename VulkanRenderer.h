#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include <stdexcept>    // for runtime error and handling exceptions in main.cpp
#include <vector>       // !! main.cpp takes it from here be careful !!

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


class VulkanRenderer
{
public:
    void run();


private: // Struct

    struct QueueFamilyIndices {
        uint32_t graphicsFamily = static_cast<uint32_t>(- 1);
        // uint32_t presentationFamily = -1;           // will be handled later when we start to use surface

        bool isValid()
        {
            return graphicsFamily >= 0
                //&& presentationFamily >= 0 - will be added later
                ;
        }
    } ;

    

private: // Functions

    // Main Structure
    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();

    // Vulkan Functions
    // - Create Functions
    void createInstance();
    void pickSuitablePhysicalDevice();
    void createLogicalDevice();
    void createSurface();

    // - Validation Layers Functions
    void setupDebugMessenger();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    // - Check Functions
    bool checkExtentionSupport(std::vector<const char*>* checkExtentions );
    bool checkValidationLayerSupport();
    bool checkIsDeviceSuitable(VkPhysicalDevice locPhysicalDevice);

    // -- Getter Functions
    QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);

private: // Variables

    // Window Variables
    GLFWwindow* window;
 
    // Vulkan Variables
    VkInstance instance;
    
    // - Device
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;   // physical device (barely used)
    VkDevice device;                                    // logical device (used in nearly everything)
    
    // - Queue
    QueueFamilyIndices queueFamilyIndices;
    VkQueue graphicsQueue;

    // - Window
    VkSurfaceKHR surface;                               // surface to present images on screen

    // - Validation Layers Variables
    VkDebugUtilsMessengerEXT debugMessenger;
    

};

