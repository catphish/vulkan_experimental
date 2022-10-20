#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct engine {
  GLFWwindow* window;
  VkInstance instance;
  VkSurfaceKHR surface;
  VkPhysicalDevice physicalDevice;
  int queueFamilyIndex;
  VkDevice device;
  VkQueue queue;
  VkExtent2D extent;

  VkSwapchainKHR swapChain;
  uint32_t swapChainImageCount;
  VkImage* swapChainImages;
  VkImageView* swapChainImageViews;
  VkFramebuffer* swapChainFramebuffers;

  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;

  VkRenderPass renderPass;

  VkCommandPool commandPool;
  VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
  VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
  VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
  VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];

  int currentFrame;
} Engine;

Engine* engineCreate(void);
void engineRun(Engine* engine);
void engineDestroy(Engine* engine);
