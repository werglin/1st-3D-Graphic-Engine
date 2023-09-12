#include "VulkanRenderer.h"

#include <iostream>

// Validation Layers Extra Functions

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    // uses built in function with this method 
    // To Check if there is a extention which is supports this, its used that way
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    // uses built in function with this method 
    // To Check if there is a extention which is supports this, its used that way
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}


// Start of renderer

void VulkanRenderer::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void VulkanRenderer::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);       // GLFW_NO_API : Defines we will not use OPENGL
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);         // GLFW_FALSE  : Defines we will handle resizing with VULKAN API


    window = glfwCreateWindow(800, 600, "Vulkan Practice", nullptr, nullptr);
}

void VulkanRenderer::initVulkan()
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickSuitablePhysicalDevice();
    createLogicalDevice();
    
}

void VulkanRenderer::mainLoop()
{
    while (!glfwWindowShouldClose(window)) {
        // checks if window is closed (reads input)
        // need to handle this to make inputs meaningful
        glfwPollEvents();                       
    }
}

void VulkanRenderer::cleanup()
{
    vkDestroyDevice(device, nullptr);   // deletes queues too 
    
    vkDestroySurfaceKHR(instance, surface, nullptr);
    
    if (enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();
}

void VulkanRenderer::createInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "MyApp";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "MyEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_1;                    // vulkan version 1.1
    
    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;

    // get required extentions

    uint32_t reqExtentionCount;
    const char** reqExtentionNames;
    reqExtentionNames = glfwGetRequiredInstanceExtensions(&reqExtentionCount); // fills the count and returnes the names

    std::vector<const char*> reqExtentions = std::vector<const char*>();
    for (size_t i = 0; i < reqExtentionCount; i++)
    {
        reqExtentions.push_back(reqExtentionNames[i]);
    }

    // important for debug and handling errors
    reqExtentions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    if (checkExtentionSupport(&reqExtentions) == false)
    {
        throw std::runtime_error("Vulkan extentions are not available !!");
    }


    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        // Why does it send the filled create info to pnext?
        instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.ppEnabledLayerNames = nullptr;
    }


    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(reqExtentions.size());
    instanceCreateInfo.ppEnabledExtensionNames = reqExtentions.data();


    VkResult res = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create instance in - ('createInstance') - function !!");
    }

}

void VulkanRenderer::pickSuitablePhysicalDevice()
{
    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("Failed to find any device which have vulkan support ( - 'pickSuitablePhysicalDevice' - )!!!");
    }

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

    for (const auto& device : physicalDevices)
    {
        if (checkIsDeviceSuitable(device))
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) 
    {   
        // if suitable queue family doesnt exist then gives this error
        throw std::runtime_error("Failed to find a suitable GPU ( - 'pickSuitablePhysicalDevice' - )!!!");
    }
}

void VulkanRenderer::createLogicalDevice()
{
    
    VkDeviceQueueCreateInfo queueInfo = {};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueCount = 1;
    queueInfo.queueFamilyIndex = this->queueFamilyIndices.graphicsFamily;
    // if queue count is > 1 , vulkan needs float array to arrange priority 
    float queuePriority = 1.0f;
    queueInfo.pQueuePriorities = &queuePriority;
    
    // dont know what it is:; will learn later from main site
    // i remembered what it is, this shit is mad but i dont realy know how to use so ill wait
    VkPhysicalDeviceFeatures physicalDeviceFeatures = {}; 

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueInfo;
    deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
    // We won't need any device specific extensions for now.
    deviceCreateInfo.enabledExtensionCount = 0;     
    // its ignored now but 
    // However, it is still a good idea to set them anyway to be compatible with older implementations:
    if (enableValidationLayers)
    {
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        deviceCreateInfo.enabledLayerCount = 0;
    }

    VkResult res = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device ( - 'createLogicalDevice' - ) !!!");
    }

    // we want only 1 queue so we get the first one (maybe will get more later on)
    vkGetDeviceQueue(device, this->queueFamilyIndices.graphicsFamily, 0, &graphicsQueue);
   
}

void VulkanRenderer::createSurface()
{
    // creates surface for every system (e.g window, linux... )
    VkResult res = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create surface !!!");
    }
}

void VulkanRenderer::setupDebugMessenger()
{
    if (!enableValidationLayers)
    {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }

}

void VulkanRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
        // - too many unusable info (maybe can be looked at when making AAA game)
        // | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT 
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT 
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT 
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT 
        | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // Optional
}

bool VulkanRenderer::checkExtentionSupport(std::vector<const char*>* checkExtentions)
{
    uint32_t avaExtentionCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &avaExtentionCount, nullptr);
    std::vector<VkExtensionProperties> avaExtentions(avaExtentionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &avaExtentionCount, avaExtentions.data());

    bool checker = false;
    for (size_t i = 0; i < (*checkExtentions).size(); i++)
    {
        for (size_t j = 0; j < avaExtentionCount; j++)
        {
            if (std::strcmp((*checkExtentions)[i], avaExtentions[j].extensionName)==0)
            {
                checker = true;
                break;
            }

        }
        if (checker == false)
        {
            return false;
        }
        checker = false;
    }
    return true;
}

bool VulkanRenderer::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    
    bool layerFound = false;
    for (const char* layerName : validationLayers) {

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
        layerFound = false;
    }

    return true;
}

bool VulkanRenderer::checkIsDeviceSuitable(VkPhysicalDevice locPhysicalDevice)
{
    /*
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(locPhysicalDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(locPhysicalDevice, &deviceFeatures);
    */

    QueueFamilyIndices gettedIndices = getQueueFamilies(locPhysicalDevice);
    if (gettedIndices.isValid())
    {
        queueFamilyIndices = gettedIndices;
    }

    return queueFamilyIndices.isValid();
}

VulkanRenderer::QueueFamilyIndices VulkanRenderer::getQueueFamilies(VkPhysicalDevice phyDevice)
{
    QueueFamilyIndices indices = {};

    uint32_t queueCount;
    vkGetPhysicalDeviceQueueFamilyProperties(phyDevice, &queueCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueProps(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(phyDevice, &queueCount, queueProps.data());

    for (uint32_t i = 0; i < queueCount; i++)
    {//VK_QUEUE_COMPUTE_BIT VK_QUEUE_TRANSFER_BIT
        if ( queueProps[i].queueCount > 0 && // can be zero so we need to check
            queueProps[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT 
                // | VK_QUEUE_COMPUTE_BIT 
                | VK_QUEUE_TRANSFER_BIT))
        {
            indices.graphicsFamily = i;
        }
    }

    return indices;
}
