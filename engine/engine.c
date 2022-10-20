#include "engine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Private function definitions

void engineCreateWindow(Engine *engine);
void engineCreateInstance(Engine *engine);
void engineCreateSurface(Engine *engine);
void enginePhysicalDeviceSelect(Engine *engine);
void engineCreateDevice(Engine *engine);
void engineCreateSwapChain(Engine *engine);
void engineCreateSwapChainImageViews(Engine *engine);
void engineCreateRenderPass(Engine *engine);
void engineCreateCommandPool(Engine *engine);
void engineCreateCommandBuffers(Engine *engine);
void engineCreateDepthResources(Engine *engine);
void engineCreateFramebuffers(Engine *engine);
void engineCreateSyncObjects(Engine *engine);
void engineCreateImage(Engine *engine, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage *image, VkDeviceMemory *imageMemory);
void engineCreateImageView(Engine *engine, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView *imageView);
void engineDestroySwapChain(Engine *engine);
void engineDrawFrame(Engine *engine);

// Public Functions

Engine *engineCreate(void) {
  Engine *engine;
  engine = malloc(sizeof(Engine));
  engineCreateWindow(engine);
  engineCreateInstance(engine);
  engineCreateSurface(engine);
  enginePhysicalDeviceSelect(engine);
  engineCreateDevice(engine);
  vkGetDeviceQueue(engine->device, engine->queueFamilyIndex, 0, &engine->queue);
  engineCreateRenderPass(engine);

  engineCreateCommandPool(engine);
  engineCreateCommandBuffers(engine);
  engineCreateSyncObjects(engine);

  engineCreateSwapChain(engine);

  return engine;
}

void engineRun(Engine *engine) {
  while (!glfwWindowShouldClose(engine->window)) {
    glfwPollEvents();
    engineDrawFrame(engine);
  }
}

void engineDestroy(Engine *engine) {
  engineDestroySwapChain(engine);

  for (int n = 0; n < MAX_FRAMES_IN_FLIGHT; n++) {
    vkDestroySemaphore(engine->device, engine->imageAvailableSemaphores[n], NULL);
    vkDestroySemaphore(engine->device, engine->renderFinishedSemaphores[n], NULL);
    vkDestroyFence(engine->device, engine->inFlightFences[n], NULL);
  }

  vkDestroyCommandPool(engine->device, engine->commandPool, NULL);
  vkDestroyRenderPass(engine->device, engine->renderPass, NULL);
  vkDestroyDevice(engine->device, NULL);
  vkDestroySurfaceKHR(engine->instance, engine->surface, NULL);
  vkDestroyInstance(engine->instance, NULL);
  glfwDestroyWindow(engine->window);
  free(engine);
}

// Private functions

void engineCreateInstance(Engine *engine) {
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

  if (vkCreateInstance(&createInfo, NULL, &engine->instance) != VK_SUCCESS) {
    printf("Failed to create instance!\n");
    exit(EXIT_FAILURE);
  }
}

void engineCreateWindow(Engine *engine) {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  engine->window = glfwCreateWindow(800, 600, "Vulkan", NULL, NULL);
  if (!engine->window) {
    printf("Failed to create window");
    exit(EXIT_FAILURE);
  }
}

void engineCreateSurface(Engine *engine) {
  if (glfwCreateWindowSurface(engine->instance, engine->window, NULL, &engine->surface) != VK_SUCCESS) {
    printf("Failed to create surface!\n");
    exit(EXIT_FAILURE);
  }
}

void enginePhysicalDeviceSelect(Engine *engine) {
  uint32_t deviceCount;
  vkEnumeratePhysicalDevices(engine->instance, &deviceCount, NULL);
  if (deviceCount == 0) {
    printf("Failed to find GPUs with Vulkan support!\n");
    exit(EXIT_FAILURE);
  }
  VkPhysicalDevice devices[deviceCount];
  vkEnumeratePhysicalDevices(engine->instance, &deviceCount, devices);

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
        vkGetPhysicalDeviceSurfaceSupportKHR(device, n, engine->surface, &presentSupport);
        if (presentSupport) {
          engine->physicalDevice = device;
          engine->queueFamilyIndex = n;
          return;
        }
      }
    }
    printf("No suitable GPU found!\n");
    exit(EXIT_FAILURE);
  }
}

void engineCreateDevice(Engine *engine) {
  const char *deviceExtensions = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

  VkDeviceQueueCreateInfo queueCreateInfo;
  memset(&queueCreateInfo, 0, sizeof(queueCreateInfo));
  float queuePriority = 1.0f;
  queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.queueFamilyIndex = engine->queueFamilyIndex;
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

  if (vkCreateDevice(engine->physicalDevice, &deviceCreateInfo, NULL, &engine->device) != VK_SUCCESS) {
    printf("Failed to create logical decvice!\n");
    exit(EXIT_FAILURE);
  }
}

void engineCreateSwapChain(Engine *engine) {
  glfwGetFramebufferSize(engine->window, &engine->extent.width, &engine->extent.height);

  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(engine->physicalDevice, engine->surface, &capabilities);

  VkSwapchainCreateInfoKHR createInfo;
  memset(&createInfo, 0, sizeof(VkSwapchainCreateInfoKHR));
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = engine->surface;
  createInfo.minImageCount = 4;
  createInfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
  createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  createInfo.imageExtent = engine->extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.preTransform = capabilities.currentTransform;
  createInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;
  if (vkCreateSwapchainKHR(engine->device, &createInfo, NULL, &engine->swapChain) != VK_SUCCESS) {
    printf("Swapchain creation failed!\n");
    exit(1);
  }
  vkGetSwapchainImagesKHR(engine->device, engine->swapChain, &engine->swapChainImageCount, NULL);
  engine->swapChainImages = malloc(engine->swapChainImageCount * sizeof(VkImage));
  vkGetSwapchainImagesKHR(engine->device, engine->swapChain, &engine->swapChainImageCount, engine->swapChainImages);

  engineCreateSwapChainImageViews(engine);
  engineCreateDepthResources(engine);
  engineCreateFramebuffers(engine);
}

uint32_t engineFindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t n = 0; n < memProperties.memoryTypeCount; n++) {
    if ((typeFilter & (1 << n)) && (memProperties.memoryTypes[n].propertyFlags & properties) == properties) {
      return n;
    }
  }
  printf("Failed to find suitable memory type!\n");
  exit(1);
}

void engineCreateImage(Engine *engine, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage *image, VkDeviceMemory *imageMemory) {
  VkImageCreateInfo imageInfo;
  memset(&imageInfo, 0, sizeof(VkImageCreateInfo));
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags = 0;  // Optional
  if (vkCreateImage(engine->device, &imageInfo, NULL, image) != VK_SUCCESS) {
    printf("Failed to create image!\n");
    exit(1);
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(engine->device, *image, &memRequirements);

  VkMemoryAllocateInfo allocInfo;
  memset(&allocInfo, 0, sizeof(VkMemoryAllocateInfo));
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = engineFindMemoryType(engine->physicalDevice, memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(engine->device, &allocInfo, NULL, imageMemory) != VK_SUCCESS) {
    printf("Failed to allocate image memory!\n");
    exit(1);
  }

  if (vkBindImageMemory(engine->device, *image, *imageMemory, 0) != VK_SUCCESS) {
    printf("Failed to bind image memory!\n");
    exit(1);
  }
}

void engineCreateImageView(Engine *engine, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView *imageView) {
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

  if (vkCreateImageView(engine->device, &viewInfo, NULL, imageView) != VK_SUCCESS) {
    printf("Failed to create image view!\n");
    exit(1);
  }
}

void engineCreateSwapChainImageViews(Engine *engine) {
  engine->swapChainImageViews = malloc(engine->swapChainImageCount * sizeof(VkImageView));
  for (int n = 0; n < engine->swapChainImageCount; n++) {
    engineCreateImageView(engine, engine->swapChainImages[n], VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, engine->swapChainImageViews + n);
  }
}

void engineCreateDepthResources(Engine *engine) {
  VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
  engineCreateImage(engine, engine->extent.width, engine->extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &engine->depthImage, &engine->depthImageMemory);
  engineCreateImageView(engine, engine->depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &engine->depthImageView);
}

void engineCreateFramebuffers(Engine *engine) {
  engine->swapChainFramebuffers = malloc(engine->swapChainImageCount * sizeof(VkFramebuffer));
  for (int n = 0; n < engine->swapChainImageCount; n++) {
    VkImageView attachments[] = {engine->swapChainImageViews[n], engine->depthImageView};
    VkFramebufferCreateInfo framebufferInfo;
    memset(&framebufferInfo, 0, sizeof(VkFramebufferCreateInfo));
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = engine->renderPass;
    framebufferInfo.attachmentCount = sizeof(attachments) / sizeof(VkImageView);
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = engine->extent.width;
    framebufferInfo.height = engine->extent.height;
    framebufferInfo.layers = 1;
    if (vkCreateFramebuffer(engine->device, &framebufferInfo, NULL, engine->swapChainFramebuffers + n) != VK_SUCCESS) {
      printf("Framebuffer creation failed!\n");
      exit(1);
    }
  }
}

void engineDestroyViews(VkDevice device, uint32_t swapChainImageCount, VkImageView *swapChainImageViews) {
  for (int i = 0; i < swapChainImageCount; i++) {
    vkDestroyImageView(device, swapChainImageViews[i], NULL);
  }
  free(swapChainImageViews);
}

void engineCreateRenderPass(Engine *engine) {
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

  if (vkCreateRenderPass(engine->device, &renderPassInfo, NULL, &engine->renderPass) != VK_SUCCESS) {
    printf("Render pass creation failed!\n");
    exit(1);
  }
}

void engineCreateCommandPool(Engine *engine) {
  VkCommandPoolCreateInfo poolInfo;
  memset(&poolInfo, 0, sizeof(VkCommandPoolCreateInfo));
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = engine->queueFamilyIndex;
  if (vkCreateCommandPool(engine->device, &poolInfo, NULL, &engine->commandPool) != VK_SUCCESS) {
    printf("Command pool creation failed!\n");
    exit(1);
  }
}

void engineCreateCommandBuffers(Engine *engine) {
  VkCommandBufferAllocateInfo allocInfo;
  memset(&allocInfo, 0, sizeof(VkCommandBufferAllocateInfo));
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = engine->commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
  if (vkAllocateCommandBuffers(engine->device, &allocInfo, engine->commandBuffers) != VK_SUCCESS) {
    printf("Command buffer creation failed!\n");
    exit(1);
  }
}

void engineDestroySwapChain(Engine *engine) {
  vkDeviceWaitIdle(engine->device);

  vkDestroyImageView(engine->device, engine->depthImageView, NULL);
  vkDestroyImage(engine->device, engine->depthImage, NULL);
  vkFreeMemory(engine->device, engine->depthImageMemory, NULL);
  for (int n = 0; n < engine->swapChainImageCount; n++) vkDestroyFramebuffer(engine->device, engine->swapChainFramebuffers[n], NULL);
  for (int n = 0; n < engine->swapChainImageCount; n++) vkDestroyImageView(engine->device, engine->swapChainImageViews[n], NULL);
  vkDestroySwapchainKHR(engine->device, engine->swapChain, NULL);
}

void engineCreateSyncObjects(Engine *engine) {
  VkSemaphoreCreateInfo semaphoreInfo;
  memset(&semaphoreInfo, 0, sizeof(VkSemaphoreCreateInfo));
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo;
  memset(&fenceInfo, 0, sizeof(VkFenceCreateInfo));
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (int n = 0; n < MAX_FRAMES_IN_FLIGHT; n++) {
    if (vkCreateSemaphore(engine->device, &semaphoreInfo, NULL, engine->imageAvailableSemaphores + n) != VK_SUCCESS) {
      printf("Failed to create semaphore!\n");
      exit(1);
    }
    if (vkCreateSemaphore(engine->device, &semaphoreInfo, NULL, engine->renderFinishedSemaphores + n) != VK_SUCCESS) {
      printf("Failed to create semaphore!\n");
      exit(1);
    }
    if (vkCreateFence(engine->device, &fenceInfo, NULL, engine->inFlightFences + n) != VK_SUCCESS) {
      printf("Failed to create fence!\n");
      exit(1);
    }
  }
}

void engineDrawFrame(Engine *engine) {
  vkWaitForFences(engine->device, 1, &engine->inFlightFences[engine->currentFrame], VK_TRUE, UINT64_MAX);
  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(engine->device, engine->swapChain, UINT64_MAX, engine->imageAvailableSemaphores[engine->currentFrame], VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    engineDestroySwapChain(engine);
    engineCreateSwapChain(engine);
    return;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    printf("Failed to acquire swap chain image!\n");
    exit(1);
  }

  VkCommandBuffer commandBuffer = engine->commandBuffers[engine->currentFrame];

  vkResetFences(engine->device, 1, &engine->inFlightFences[engine->currentFrame]);
  vkResetCommandBuffer(commandBuffer, 0);

  VkCommandBufferBeginInfo beginInfo;
  memset(&beginInfo, 0, sizeof(VkCommandBufferBeginInfo));
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    printf("Begin command buffer failed!\n");
    exit(1);
  }

  VkRenderPassBeginInfo renderPassInfo;
  memset(&renderPassInfo, 0, sizeof(VkRenderPassBeginInfo));
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = engine->renderPass;
  renderPassInfo.framebuffer = engine->swapChainFramebuffers[imageIndex];
  renderPassInfo.renderArea.offset.x = 0;
  renderPassInfo.renderArea.offset.y = 0;
  renderPassInfo.renderArea.extent = engine->extent;

  VkClearValue clearValues[2];
  memset(&clearValues, 0, sizeof(clearValues));
  clearValues[0].color = (VkClearColorValue){{0.0f, 0.0f, 0.0f, 1.0f}};
  clearValues[1].depthStencil = (VkClearDepthStencilValue){1.0f, 0};
  renderPassInfo.clearValueCount = 2;
  renderPassInfo.pClearValues = clearValues;

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport;
  memset(&viewport, 0, sizeof(VkViewport));
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)engine->extent.width;
  viewport.height = (float)engine->extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor;
  memset(&scissor, 0, sizeof(VkRect2D));
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  scissor.extent = engine->extent;
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  vkCmdEndRenderPass(commandBuffer);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    printf("Failed to record command buffer!\n");
    exit(1);
  }

  VkSubmitInfo submitInfo;
  memset(&submitInfo, 0, sizeof(VkSubmitInfo));
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {engine->imageAvailableSemaphores[engine->currentFrame]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
  VkSemaphore signalSemaphores[] = {engine->renderFinishedSemaphores[engine->currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;
  if (vkQueueSubmit(engine->queue, 1, &submitInfo, engine->inFlightFences[engine->currentFrame]) != VK_SUCCESS) {
    printf("Failed to submit draw command buffer!\n");
    exit(1);
  }

  VkPresentInfoKHR presentInfo;
  memset(&presentInfo, 0, sizeof(VkPresentInfoKHR));
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;
  VkSwapchainKHR swapChains[] = {engine->swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;
  result = vkQueuePresentKHR(engine->queue, &presentInfo);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    engineDestroySwapChain(engine);
    engineCreateSwapChain(engine);
  } else if (result != VK_SUCCESS) {
    printf("Failed to present swap chain image!\n");
    exit(1);
  }

  engine->currentFrame++;
  engine->currentFrame %= MAX_FRAMES_IN_FLIGHT;
}
