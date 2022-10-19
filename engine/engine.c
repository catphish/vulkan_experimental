#include "engine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Private function definitions

void engineCreateWindow(GLFWwindow **window);
void engineCreateInstance(VkInstance *instance);
void engineCreateSurface(VkInstance instance, GLFWwindow *window, VkSurfaceKHR *surface);
void enginePhysicalDeviceSelect(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice *physicalDevice, uint32_t *queueFamilyIndex);
void engineDeviceCreate(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkDevice *device);
void engineSwapChainCreate(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkExtent2D extent, VkSwapchainKHR *swapChain, uint32_t *swapChainImageCount, VkImage **swapChainImages);
void engineCreateView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView *imageView);
void engineCreateViews(VkDevice device, uint32_t swapChainImageCount, VkImage *swapChainImages, VkImageView **swapChainImageViews);
void engineDestroyViews(VkDevice device, uint32_t swapChainImageCount, VkImageView *swapChainImageViews);
void engineCreateRenderPass(VkDevice device, VkRenderPass *renderPass);

// Public Functions

Engine *engineCreate(void) {
  int queueFamilyIndex;
  Engine *engine;
  engine = malloc(sizeof(Engine));
  engineCreateWindow(&engine->window);
  engineCreateInstance(&engine->instance);
  engineCreateSurface(engine->instance, engine->window, &engine->surface);
  enginePhysicalDeviceSelect(engine->instance, engine->surface, &engine->physicalDevice, &queueFamilyIndex);
  engineDeviceCreate(engine->physicalDevice, queueFamilyIndex, &engine->device);
  vkGetDeviceQueue(engine->device, queueFamilyIndex, 0, &engine->queue);
  glfwGetFramebufferSize(engine->window, &engine->extent.width, &engine->extent.height);
  engineSwapChainCreate(engine->physicalDevice, engine->device, engine->surface, engine->extent, &engine->swapChain, &engine->swapChainImageCount, &engine->swapChainImages);
  engineCreateViews(engine->device, engine->swapChainImageCount, engine->swapChainImages, &engine->swapChainImageViews);
  engineCreateRenderPass(engine->device, &engine->renderPass);
  return engine;
}

void engineRun(Engine *engine) {
  while (!glfwWindowShouldClose(engine->window)) {
    glfwPollEvents();
  }
}

void engineDestroy(Engine *engine) {
  vkDestroyRenderPass(engine->device, engine->renderPass, NULL);
  engineDestroyViews(engine->device, engine->swapChainImageCount, engine->swapChainImageViews);
  vkDestroyDevice(engine->device, NULL);
  vkDestroySurfaceKHR(engine->instance, engine->surface, NULL);
  vkDestroyInstance(engine->instance, NULL);
  glfwDestroyWindow(engine->window);
  free(engine);
}

// Private functions

void engineCreateInstance(VkInstance *instance) {
  const char *validationLayers = "VK_LAYER_KHRONOS_validation";

  uint32_t glfwExtensionCount;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  VkApplicationInfo appInfo;
  memset(&appInfo, 0, sizeof(appInfo));
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Vulkan";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo;
  memset(&createInfo, 0, sizeof(createInfo));
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledExtensionCount = glfwExtensionCount;
  createInfo.ppEnabledExtensionNames = glfwExtensions;
  createInfo.enabledLayerCount = 1;
  createInfo.ppEnabledLayerNames = &validationLayers;

  if (vkCreateInstance(&createInfo, NULL, instance) != VK_SUCCESS) {
    printf("Failed to create instance!\n");
    exit(EXIT_FAILURE);
  }
}

void engineCreateWindow(GLFWwindow **window) {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  *window = glfwCreateWindow(800, 600, "Vulkan", NULL, NULL);
  if (!window) {
    printf("Failed to create window");
    exit(EXIT_FAILURE);
  }
}

void engineCreateSurface(VkInstance instance, GLFWwindow *window, VkSurfaceKHR *surface) {
  if (glfwCreateWindowSurface(instance, window, NULL, surface) != VK_SUCCESS) {
    printf("Failed to create surface!\n");
    exit(EXIT_FAILURE);
  }
}

void enginePhysicalDeviceSelect(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice *physicalDevice, uint32_t *queueFamilyIndex) {
  uint32_t deviceCount;
  vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
  if (deviceCount == 0) {
    printf("Failed to find GPUs with Vulkan support!\n");
    exit(EXIT_FAILURE);
  }
  VkPhysicalDevice devices[deviceCount];
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

  for (int n = 0; n < deviceCount; n++) {
    VkPhysicalDevice device = devices[n];
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
    if (queueFamilyCount == 0) continue;
    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);
    for (int n = 0; n < queueFamilyCount; n++) {
      VkQueueFamilyProperties queueFamily = queueFamilies[n];
      int required_queue_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
      if (queueFamily.queueFlags & required_queue_flags == required_queue_flags) {
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, n, surface, &presentSupport);
        if (presentSupport) {
          *physicalDevice = device;
          *queueFamilyIndex = n;
          return;
        }
      }
    }
    printf("No suitable GPU found!\n");
    exit(EXIT_FAILURE);
  }
}

void engineDeviceCreate(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkDevice *device) {
  const char *deviceExtensions = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

  VkDeviceQueueCreateInfo queueCreateInfo;
  memset(&queueCreateInfo, 0, sizeof(queueCreateInfo));
  float queuePriority = 1.0f;
  queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
  queueCreateInfo.queueCount = 1;
  queueCreateInfo.pQueuePriorities = &queuePriority;

  VkPhysicalDeviceFeatures deviceFeatures;
  memset(&deviceFeatures, 0, sizeof(deviceFeatures));
  deviceFeatures.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo deviceCreateInfo;
  memset(&deviceCreateInfo, 0, sizeof(deviceCreateInfo));
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
  deviceCreateInfo.queueCreateInfoCount = 1;
  deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
  deviceCreateInfo.enabledExtensionCount = 0;
  deviceCreateInfo.enabledExtensionCount = 1;
  deviceCreateInfo.ppEnabledExtensionNames = &deviceExtensions;

  if (vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, device) != VK_SUCCESS) {
    printf("Failed to create logical decvice!\n");
    exit(EXIT_FAILURE);
  }
}

void engineSwapChainCreate(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkExtent2D extent, VkSwapchainKHR *swapChain, uint32_t *swapChainImageCount, VkImage **swapChainImages) {
  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

  VkSwapchainCreateInfoKHR createInfo;
  memset(&createInfo, 0, sizeof(VkSwapchainCreateInfoKHR));
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;
  createInfo.minImageCount = 4;
  createInfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
  createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.preTransform = capabilities.currentTransform;
  createInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;
  if (vkCreateSwapchainKHR(device, &createInfo, NULL, swapChain) != VK_SUCCESS) {
    printf("Swapchain creation failed!\n");
    exit(1);
  }
  vkGetSwapchainImagesKHR(device, *swapChain, swapChainImageCount, NULL);
  *swapChainImages = malloc(*swapChainImageCount * sizeof(VkImage));
  vkGetSwapchainImagesKHR(device, *swapChain, swapChainImageCount, *swapChainImages);
}

void engineCreateView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView *imageView) {
  VkImageViewCreateInfo viewInfo;
  memset(&viewInfo, 0, sizeof(VkImageViewCreateInfo));
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  if (vkCreateImageView(device, &viewInfo, NULL, imageView) != VK_SUCCESS) {
    printf("Failed to create image view!\n");
    exit(1);
  }
}

void engineCreateViews(VkDevice device, uint32_t swapChainImageCount, VkImage *swapChainImages, VkImageView **swapChainImageViews) {
  *swapChainImageViews = malloc(swapChainImageCount * sizeof(VkImageView));
  for (int i = 0; i < swapChainImageCount; i++) {
    engineCreateView(device, swapChainImages[i], VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, *swapChainImageViews + i);
  }
}

void engineDestroyViews(VkDevice device, uint32_t swapChainImageCount, VkImageView *swapChainImageViews) {
  for (int i = 0; i < swapChainImageCount; i++) {
    vkDestroyImageView(device, swapChainImageViews[i], NULL);
  }
  free(swapChainImageViews);
}

void engineCreateRenderPass(VkDevice device, VkRenderPass *renderPass) {
  VkAttachmentDescription colorAttachment;
  memset(&colorAttachment, 0, sizeof(VkAttachmentDescription));
  colorAttachment.format = VK_FORMAT_B8G8R8A8_SRGB;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentDescription depthAttachment;
  memset(&depthAttachment, 0, sizeof(VkAttachmentDescription));
  depthAttachment.format = VK_FORMAT_D32_SFLOAT;
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference colorAttachmentRef;
  memset(&colorAttachmentRef, 0, sizeof(VkAttachmentReference));
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef;
  memset(&depthAttachmentRef, 0, sizeof(VkAttachmentReference));
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass;
  memset(&subpass, 0, sizeof(VkSubpassDescription));
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  VkSubpassDependency dependency;
  memset(&dependency, 0, sizeof(VkSubpassDependency));
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  VkAttachmentDescription attachments[2] = {colorAttachment, depthAttachment};
  VkRenderPassCreateInfo renderPassInfo;
  memset(&renderPassInfo, 0, sizeof(VkRenderPassCreateInfo));
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = sizeof(attachments) / sizeof(VkAttachmentDescription);
  renderPassInfo.pAttachments = attachments;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(device, &renderPassInfo, NULL, renderPass) != VK_SUCCESS) {
    printf("Render pass creation failed!\n");
    exit(1);
  }
}
