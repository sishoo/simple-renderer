#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "bootstrap/VkBootstrap.h"

typedef struct
{
        VkInstance instance;
        VkPhysicalDevice pdevice;
        VkDevice device;
        VkSwapchainKHR swapchain;
        uint32_t nswapchain_images;
        VkImage *swapchain_images;      
        VkQueue queue;

        VkCommandPool command_pool;

        

} renderer_t; 
