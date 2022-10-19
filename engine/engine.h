#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef struct engine {
  GLFWwindow* window;
  VkInstance instance;
  VkSurfaceKHR surface;
  VkPhysicalDevice physicalDevice;
  VkDevice device;
  VkQueue queue;
  VkExtent2D extent;
  VkSwapchainKHR swapChain;
  uint32_t swapChainImageCount;
  VkImage* swapChainImages;
  VkImageView* swapChainImageViews;
  VkRenderPass renderPass;
} Engine;

Engine* engineCreate(void);
void engineRun(Engine* engine);
void engineDestroy(Engine* engine);
