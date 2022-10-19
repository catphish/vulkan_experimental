#include "window.h"

#include <stdio.h>
#include <stdlib.h>

GLFWwindow *window_create() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow *window = glfwCreateWindow(800, 600, "Vulkan", NULL, NULL);
  if (!window) {
    printf("Failed to create window");
    exit(1);
  }
  return window;
}