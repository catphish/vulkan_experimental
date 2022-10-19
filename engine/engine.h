#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct engine {
  GLFWwindow* window;
  VkInstance instance;
  VkSurfaceKHR surface;
  VkPhysicalDevice physicalDevice;
  VkDevice device;
  VkQueue queue;
};

struct engine* engine_create();
void engine_destroy(struct engine* engine);
void engine_run(struct engine* engine);
